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
	class VulkanRenderTarget;
	class VulkanStateManager;
	class VulkanBuffer;
	class VulkanUniformBuffer;
	class VulkanFence;
	class VulkanDescriptorSetAllocator;

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

		class VulkanFence* m_fence = nullptr;
		class VulkanCommandBufferManager* m_manager;
		Vector<UniformBuffer> m_uniform_buffers;
		Vector<RHIObject*> m_stagging;
		State m_state = State::Unused;
		const RHIContextFlags m_flags;

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
		CriticalSection m_critical;
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

	class VulkanContext : public RHIContext
	{
	private:
		VulkanStateManager* m_state_manager;
		VulkanCommandHandle* m_cmd = nullptr;
		RHIContextFlags m_flags;

		void destroy() override;

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

		VulkanContext& draw(usize vertex_count, usize vertices_offset, usize instances) override;
		VulkanContext& draw_indexed(usize indices_count, usize indices_offset, usize vertices_offset,
		                            usize instances) override;

		VulkanContext& draw_mesh(u32 x, u32 y, u32 z) override;
		VulkanContext& dispatch(u32 group_x, u32 group_y, u32 group_z) override;
		VulkanContext& trace_rays(u32 width, u32 height, u32 depth, u64 raygen = 0, const RHIRange& miss = {},
		                          const RHIRange& hit = {}, const RHIRange& callable = {}) override;

		VulkanContext& push_debug_stage(const char* stage) override;
		VulkanContext& pop_debug_stage() override;

		VulkanContext& clear_rtv(RHIRenderTargetView* rtv, const vk::ClearColorValue& color);
		VulkanContext& clear_rtv(RHIRenderTargetView* rtv, f32 r, f32 g, f32 b, f32 a) override;
		VulkanContext& clear_urtv(RHIRenderTargetView* rtv, u32 r, u32 g, u32 b, u32 a) override;
		VulkanContext& clear_irtv(RHIRenderTargetView* rtv, i32 r, i32 g, i32 b, i32 a) override;
		VulkanContext& clear_dsv(RHIDepthStencilView* dsv, f32 depth, u8 stencil) override;

		VulkanContext& update_buffer(RHIBuffer* buffer, usize offset, usize size, const u8* data) override;
		VulkanContext& update_scalar(const void* data, usize size, usize offset, u8 buffer_index) override;

		VulkanContext& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, usize size,
		                              usize buffer_width, usize buffer_height) override;

		VulkanContext& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, usize size, usize src_offset,
		                                     usize dst_offset) override;

		VulkanContext& copy_texture_to_buffer(RHITexture* texture, u8 mip_level, u16 array_slice,
		                                      const Vector3u& offset, const Vector3u& extent, RHIBuffer* buffer,
		                                      usize buffer_offset) override;

		VulkanContext& copy_buffer_to_texture(RHIBuffer* buffer, usize buffer_offset, RHITexture* texture, u8 mip_level,
		                                      u16 array_slice, const Vector3u& offset, const Vector3u& extent) override;

		VulkanContext& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                       const RHITextureRegion& dst_region) override;

		VulkanContext& depth_state(const RHIDepthState& state) override;
		VulkanContext& stencil_state(const RHIStencilState& state) override;
		VulkanContext& blending_state(const RHIBlendingState& state) override;
		VulkanContext& rasterizer_state(const RHIRasterizerState& state) override;
		VulkanContext& primitive_topology(RHIPrimitiveTopology topology) override;
		VulkanContext& polygon_mode(RHIPolygonMode mode) override;
		VulkanContext& cull_mode(RHICullMode mode) override;
		VulkanContext& front_face(RHIFrontFace face) override;
		VulkanContext& write_mask(RHIColorComponent mask) override;
		VulkanContext& shading_rate(RHIShadingRate rate, RHIShadingRateCombiner* combiners) override;

		VulkanContext& bind_vertex_attribute(RHIVertexSemantic semantic, RHIVertexFormat format, u8 stream,
		                                     u16 offset) override;
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
		inline VulkanStateManager* state() const { return m_state_manager; }
	};
}// namespace Trinex
