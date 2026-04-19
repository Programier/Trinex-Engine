#pragma once
#include <Core/etl/critical_section.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/context.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_thread_local.hpp>

namespace Trinex
{
	class RHIObject;
	class VulkanBuffer;
	class VulkanUniformBuffer;
	class VulkanFence;
	class VulkanDescriptorSetAllocator;
	class VulkanCommandHandle;
	class VulkanSampler;
	class VulkanTexture;
	class VulkanTextureSRV;
	class VulkanTextureUAV;
	class VulkanTextureRTV;
	class VulkanTextureDSV;
	class VulkanUniformBuffer;
	class VulkanPipeline;
	class VulkanContext;
	struct VulkanVertexAttribute;

	template<typename T>
	class VulkanResourceState
	{
	private:
		Vector<T> m_resources;
		Vector<u8> m_dirty_flags;

		void resize(usize index)
		{
			usize size = index + 1;
			if (size > m_resources.size())
			{
				m_resources.resize(size);
				m_dirty_flags.resize((size >> 3) + 1, 0);
			}
		}

	public:
		T resource(usize index) const { return m_resources[index]; }
		bool is_dirty(usize index) const { return m_dirty_flags[index >> 3] & (1 << (index % 8)); }
		usize size() const { return m_resources.size(); }

		template<typename Resource>
		void bind(Resource&& resource, usize index)
		{
			resize(index);
			if (m_resources[index] != resource)
			{
				m_resources[index] = std::forward<Resource>(resource);
				make_dirty(index);
			}
		}

		void flush() { std::memset(m_dirty_flags.data(), 0, m_dirty_flags.size()); }
		void make_dirty() { std::memset(m_dirty_flags.data(), 255, m_dirty_flags.size()); }
		void make_dirty(usize index) { m_dirty_flags[index >> 3] |= (1 << (index % 8)); }
	};

	class VulkanCommandHandle final : public vk::CommandBuffer, public RHICommandHandle
	{
	private:
		enum class State
		{
			Unused,
			Active,
			Pending,
			Submitted,
		};

		struct UniformBuffer {
			VulkanUniformBuffer* m_uniform_buffer_head     = nullptr;
			VulkanUniformBuffer** m_uniform_buffer_current = &m_uniform_buffer_head;

			UniformBuffer() = default;
			trinex_non_copyable(UniformBuffer);

			inline UniformBuffer(UniformBuffer&& buffer) : m_uniform_buffer_head(buffer.m_uniform_buffer_head)
			{
				if (&buffer.m_uniform_buffer_head != buffer.m_uniform_buffer_current)
				{
					m_uniform_buffer_current = buffer.m_uniform_buffer_current;
				}

				buffer.m_uniform_buffer_head    = nullptr;
				buffer.m_uniform_buffer_current = nullptr;
			}

			VulkanUniformBuffer* request_uniform_page(usize size);
			UniformBuffer& reset();
			UniformBuffer& flush();
		};

	private:
		class VulkanFence* m_fence = nullptr;
		class VulkanCommandBufferManager* m_manager;
		Vector<UniformBuffer> m_uniform_buffers;
		Vector<RHIObject*> m_stagging;
		State m_state = State::Unused;
		const RHIContextFlags m_flags;

	private:
		VulkanCommandHandle& release_transient_resources();

	public:
		VulkanCommandHandle(class VulkanCommandBufferManager* manager, RHIContextFlags flags);
		VulkanCommandHandle(const VulkanCommandHandle&) = delete;
		VulkanCommandHandle(VulkanCommandHandle&&)      = delete;
		~VulkanCommandHandle();

		VulkanCommandHandle& operator=(const VulkanCommandHandle&) = delete;
		VulkanCommandHandle& operator=(VulkanCommandHandle&&)      = delete;

		VulkanCommandHandle& refresh_fence_status();
		VulkanCommandHandle& begin(const vk::CommandBufferBeginInfo& info = {});
		VulkanCommandHandle& end();
		VulkanCommandHandle& enqueue();
		VulkanCommandHandle& wait();
		VulkanCommandHandle& flush_uniforms();

		void destroy() override;

		inline VulkanUniformBuffer* request_uniform_page(usize size, u8 index)
		{
			if (m_uniform_buffers.size() <= index)
				m_uniform_buffers.resize(index + 1);
			return m_uniform_buffers[index].request_uniform_page(size);
		}

		inline VulkanCommandHandle& add_stagging(RHIObject* object)
		{
			m_stagging.push_back(object);
			return *this;
		}

		inline bool is_unused() const { return m_state == State::Unused; }
		inline bool is_active() const { return m_state == State::Active; }
		inline bool is_pending() const { return m_state == State::Pending; }
		inline bool is_submitted() const { return m_state == State::Submitted; }
		inline bool is_secondary() const { return m_flags & RHIContextFlags::Secondary; }
		inline bool is_primary() const { return (m_flags & RHIContextFlags::Secondary) == RHIContextFlags::Undefined; }

		inline VulkanFence* fence() const { return m_fence; }
	};

	class VulkanCommandBufferManager : public VulkanThreadLocal
	{
	private:
		CriticalSectionRecursive m_critical;
		vk::CommandPool m_pool;
		Deque<VulkanCommandHandle*> m_primary;
		Deque<VulkanCommandHandle*> m_secondary;


	public:
		VulkanCommandBufferManager();
		~VulkanCommandBufferManager();

		static VulkanCommandBufferManager* instance();

		VulkanCommandHandle* request_handle(RHIContextFlags flags = RHIContextFlags::Undefined);
		VulkanCommandBufferManager& return_handle(VulkanCommandHandle* handle);

		inline vk::CommandPool command_pool() const { return m_pool; }
	};

	class VulkanContext final : public RHIContext
	{
	public:
		enum DirtyFlags : u32
		{
			RenderTarget      = 1 << 0,
			Pipeline          = 1 << 1,
			DepthStencilState = 1 << 2,
			BlendingState     = 1 << 3,
			RasterizerState   = 1 << 4,
			DepthBias         = 1 << 5,
			Viewport          = 1 << 6,
			Scissor           = 1 << 7,

			GeneralMask  = Viewport | Scissor,
			GraphicsMask = RenderTarget | Pipeline | DepthStencilState | BlendingState | RasterizerState,
			ComputeMask  = Pipeline,
		};

		struct CombinedImage {
			VulkanTextureSRV* srv;
			vk::Sampler sampler;
		};

		struct UniformBuffer {
			vk::Buffer buffer;
			u32 size;
			u32 offset;

			UniformBuffer(vk::Buffer buffer = {}, u32 size = 0, u32 offset = 0) : buffer(buffer), size(size), offset(offset) {}

			inline bool operator==(const UniformBuffer& other) const
			{
				return buffer == other.buffer && offset == other.offset && size == other.size;
			}

			inline bool operator!=(const UniformBuffer& other) const { return !((*this) == other); }
		};

		struct Framebuffer {
			vk::Format formats[6] = {vk::Format::eUndefined};
			Vector2u16 size       = {0, 0};

			vk::PipelineRenderingCreateInfo pipeline_create_info() const;
		};

		struct VertexAttribute {
			u16 stream;
			u16 offset;
			RHIVertexFormat format;

			inline bool operator==(const VertexAttribute& other) const
			{
				return stream == other.stream && offset == other.offset && format == other.format;
			}

			inline bool operator!=(const VertexAttribute& other) const { return !((*this) == other); }
		};

		struct VertexStream {
			vk::VertexInputRate rate;
			u16 stride;

			inline bool operator==(const VertexStream& other) const { return stride == other.stride && rate == other.rate; }
			inline bool operator!=(const VertexStream& other) const { return !((*this) == other); }
		};

	private:
		VulkanCommandHandle* m_cmd = nullptr;
		RHIContextFlags m_flags;

		u32 m_dirty_flags;

		Framebuffer m_framebuffer;
		RHIViewport m_viewport;
		RHIScissor m_scissor;
		RHITopology m_topology;

		struct {
			float constant;
			float clamp;
			float slope;
		} m_depth_bias;

		VulkanPipeline* m_pipeline = nullptr;

		union PipelineState
		{
			struct State {
				RHIDepthStencilState depth_stencil;
				RHIBlendingState blending;
				RHIRasterizerState rasterizer;
			} state;

			u128 id;
			PipelineState() : id(0) {}

			inline State* operator->() { return &state; }
			inline const State* operator->() const { return &state; }
		} m_pipeline_state;

		static_assert(sizeof(PipelineState) <= 16);

	private:
		void destroy() override;

		VulkanContext& flush_state(u32 mask);
		VulkanContext& bind_framebuffer(const Framebuffer& fb);

	public:
		VulkanResourceState<UniformBuffer> uniform_buffers;
		VulkanResourceState<vk::Buffer> storage_buffers;
		VulkanResourceState<vk::Buffer> uniform_texel_buffers;
		VulkanResourceState<vk::Buffer> storage_texel_buffers;
		VulkanResourceState<vk::Sampler> samplers;
		VulkanResourceState<vk::AccelerationStructureKHR> acceleration_structures;
		VulkanResourceState<VulkanTextureSRV*> srv_images;
		VulkanResourceState<VulkanTextureUAV*> uav_images;
		VulkanResourceState<VertexStream> vertex_streams;
		VulkanResourceState<VertexAttribute> vertex_attributes;

	public:
		VulkanCommandHandle* flush_graphics();
		VulkanCommandHandle* flush_compute();
		VulkanCommandHandle* flush_raytrace();
		VulkanContext& reset();
		VulkanContext& copy_state(VulkanContext* src, usize dirty_mask = ~0ULL);

		vk::PipelineVertexInputStateCreateInfo create_vertex_input(VulkanVertexAttribute* attributes, usize count);
		u128 graphics_pipeline_id(VulkanVertexAttribute* attributes, usize count) const;

		inline VulkanCommandHandle* flush_graphics(RHITopology topology)
		{
			if (topology != m_topology)
			{
				m_topology = topology;
			}

			return flush_graphics();
		}

		inline u128 pipeline_state_id() const { return m_pipeline_state.id; }
		inline u32 dirty_flags() const { return m_dirty_flags; }
		inline bool is_dirty(u32 flags) const { return (m_dirty_flags & flags); }
		inline u32 add_dirty(u32 flags) { return (m_dirty_flags |= flags); }
		inline u32 remove_dirty(u32 flags) { return (m_dirty_flags &= ~flags); }
		inline VulkanPipeline* pipeline() const { return m_pipeline; }
		inline const Framebuffer& framebuffer() const { return m_framebuffer; }
		inline RHITopology primitive_topology() const { return m_topology; }
		inline const RHIDepthStencilState& depth_stencil_state() const { return m_pipeline_state->depth_stencil; }
		inline const RHIBlendingState& blending_state() const { return m_pipeline_state->blending; }
		inline RHIPolygonMode polygon_mode() const { return m_pipeline_state->rasterizer.polygon_mode; }
		inline RHICullMode cull_mode() const { return m_pipeline_state->rasterizer.cull_mode; }
		inline RHIFrontFace front_face() const { return m_pipeline_state->rasterizer.front_face; }
		inline RHIColorComponent write_mask() const { return m_pipeline_state->blending.write_mask; }

	public:
		VulkanContext(RHIContextFlags flags);
		~VulkanContext();

		VulkanContext& begin(const RHIContextInheritanceInfo* inheritance = nullptr) override;
		VulkanCommandHandle* end() override;

		VulkanContext& begin_rendering(const RHIRenderingInfo& info) override;
		VulkanContext& end_rendering() override;

		VulkanContext& execute(RHICommandHandle* handle) override;

		VulkanContext& viewport(const RHIViewport& viewport) override;
		VulkanContext& scissor(const RHIScissor& scissor) override;

		VulkanContext& draw(RHITopology topology, usize vertex_count, usize vertices_offset, usize instances,
		                    usize first_instance) override;
		VulkanContext& draw_indexed(RHITopology topology, usize indices_count, usize indices_offset, usize vertices_offset,
		                            usize instances, usize first_instance) override;
		VulkanContext& draw_indirect(RHITopology topology, const RHIBufferAddress& args, u32 count, u32 stride) override;
		VulkanContext& draw_indirect(RHITopology topology, const RHIBufferAddress& args, const RHIBufferAddress& count,
		                             u32 max_count, u32 stride) override;
		VulkanContext& draw_indexed_indirect(RHITopology topology, const RHIBufferAddress& args, uint32_t count,
		                                     uint32_t stride) override;
		VulkanContext& draw_indexed_indirect(RHITopology topology, const RHIBufferAddress& args, const RHIBufferAddress& count,
		                                     u32 max_count, uint32_t stride) override;

		VulkanContext& draw_mesh(u32 x, u32 y, u32 z) override;

		VulkanContext& dispatch(Vector3u groups, Vector3u base) override;
		VulkanContext& dispatch_indirect(const RHIBufferAddress& args) override;

		VulkanContext& trace_rays(u32 width, u32 height, u32 depth, u64 raygen = 0, const RHIRange& miss = {},
		                          const RHIRange& hit = {}, const RHIRange& callable = {}) override;

		VulkanContext& push_debug_stage(const char* stage) override;
		VulkanContext& pop_debug_stage() override;

		VulkanContext& clear_rtv(RHIRenderTargetView* rtv, const vk::ClearColorValue& color);
		VulkanContext& clear_rtv(RHIRenderTargetView* rtv, f32 r, f32 g, f32 b, f32 a) override;
		VulkanContext& clear_urtv(RHIRenderTargetView* rtv, u32 r, u32 g, u32 b, u32 a) override;
		VulkanContext& clear_irtv(RHIRenderTargetView* rtv, i32 r, i32 g, i32 b, i32 a) override;
		VulkanContext& clear_dsv(RHIDepthStencilView* dsv, f32 depth, u8 stencil) override;

		VulkanContext& update_scalar(const void* data, usize size, usize offset, u8 buffer_index) override;

		VulkanContext& memset(RHIBuffer* dst, usize size, usize offset, u32 value = 0) override;

		VulkanContext& update(RHIBuffer* dst, const void* src, const RHIBufferCopy& region) override;
		VulkanContext& update(RHITexture* dst, const RHITextureRegion& dst_region, const void* src,
		                      const RHIBufferTextureCopy& src_region) override;

		VulkanContext& copy(RHIBuffer* dst, RHIBuffer* src, const RHIBufferCopy& region) override;

		VulkanContext& copy(RHITexture* dst, const RHITextureRegion& dst_region, RHITexture* src,
		                    const RHITextureRegion& src_region) override;

		VulkanContext& copy(RHIBuffer* dst, const RHIBufferTextureCopy& dst_region, RHITexture* src,
		                    const RHITextureRegion& src_region) override;

		VulkanContext& copy(RHITexture* dst, const RHITextureRegion& dst_region, RHIBuffer* src,
		                    const RHIBufferTextureCopy& src_region) override;

		VulkanContext& depth_stencil_state(const RHIDepthStencilState& state) override;
		VulkanContext& blending_state(const RHIBlendingState& state) override;
		VulkanContext& rasterizer_state(const RHIRasterizerState& state) override;

		VulkanContext& depth_bias(float constant = 0.0f, float clamp = 0.0f, float slope = 0.0f) override;

		VulkanContext& bind_vertex_attribute(RHISemantic semantic, RHIVertexFormat format, u8 stream, u16 offset) override;
		VulkanContext& bind_vertex_buffer(RHIBuffer* buffer, usize byte_offset, u16 stride, u8 stream,
		                                  RHIVertexInputRate rate) override;
		VulkanContext& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format, usize byte_offset = 0) override;
		VulkanContext& bind_uniform_buffer(RHIBuffer* buffer, u8 slot) override;
		VulkanContext& bind_uniform_buffer(VulkanBuffer* buffer, u32 size, u32 offset, u8 slot);
		VulkanContext& bind_srv(RHIShaderResourceView* view, u8 slot) override;
		VulkanContext& bind_uav(RHIUnorderedAccessView* view, u8 slot) override;
		VulkanContext& bind_sampler(RHISampler* view, u8 slot) override;
		VulkanContext& bind_pipeline(RHIPipeline* pipeline) override;
		VulkanContext& bind_acceleration(RHIAccelerationStructure* acceleration, u8 slot) override;

		VulkanContext& barrier(RHITexture* texture, RHIAccess dst_access) override;
		VulkanContext& barrier(RHIBuffer* buffer, RHIAccess dst_access) override;

		VulkanContext& begin_timestamp(RHITimestamp* timestamp) override;
		VulkanContext& end_timestamp(RHITimestamp* timestamp) override;

		VulkanContext& begin_statistics(RHIPipelineStatistics* stats) override;
		VulkanContext& end_statistics(RHIPipelineStatistics* stats) override;

		inline bool is_secondary() const { return m_flags & RHIContextFlags::Secondary; }
		inline VulkanCommandHandle* handle() const { return m_cmd; }
	};
}// namespace Trinex
