#pragma once
#include <Core/etl/critical_section.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/context.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_thread_local.hpp>

namespace Engine
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

			VulkanUniformBuffer* request_uniform_page(size_t size);
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

		inline VulkanUniformBuffer* request_uniform_page(size_t size, byte index)
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

		VulkanContext& draw(size_t vertex_count, size_t vertices_offset, size_t instances) override;
		VulkanContext& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                            size_t instances) override;

		VulkanContext& draw_mesh(uint32_t x, uint32_t y, uint32_t z) override;
		VulkanContext& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
		VulkanContext& trace_rays(uint32_t width, uint32_t height, uint32_t depth, uint64_t raygen = 0, const RHIRange& miss = {},
		                          const RHIRange& hit = {}, const RHIRange& callable = {}) override;

		VulkanContext& push_debug_stage(const char* stage) override;
		VulkanContext& pop_debug_stage() override;

		VulkanContext& clear_rtv(RHIRenderTargetView* rtv, const vk::ClearColorValue& color);
		VulkanContext& clear_rtv(RHIRenderTargetView* rtv, float_t r, float_t g, float_t b, float_t a) override;
		VulkanContext& clear_urtv(RHIRenderTargetView* rtv, uint_t r, uint_t g, uint_t b, uint_t a) override;
		VulkanContext& clear_irtv(RHIRenderTargetView* rtv, int_t r, int_t g, int_t b, int_t a) override;
		VulkanContext& clear_dsv(RHIDepthStencilView* dsv, float_t depth, byte stencil) override;

		VulkanContext& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) override;
		VulkanContext& update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;

		VulkanContext& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
		                              size_t buffer_width, size_t buffer_height) override;

		VulkanContext& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset,
		                                     size_t dst_offset) override;

		VulkanContext& copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice,
		                                      const Vector3u& offset, const Vector3u& extent, RHIBuffer* buffer,
		                                      size_t buffer_offset) override;

		VulkanContext& copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture, uint8_t mip_level,
		                                      uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) override;

		VulkanContext& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                       const RHITextureRegion& dst_region) override;

		VulkanContext& depth_state(const RHIDepthState& state) override;
		VulkanContext& stencil_state(const RHIStencilState& state) override;
		VulkanContext& blending_state(const RHIBlendingState& state) override;
		VulkanContext& primitive_topology(RHIPrimitiveTopology topology) override;
		VulkanContext& polygon_mode(RHIPolygonMode mode) override;
		VulkanContext& cull_mode(RHICullMode mode) override;
		VulkanContext& front_face(RHIFrontFace face) override;
		VulkanContext& write_mask(RHIColorComponent mask) override;
		VulkanContext& shading_rate(RHIShadingRate rate, RHIShadingRateCombiner* combiners) override;

		VulkanContext& bind_vertex_attribute(RHIVertexSemantic semantic, RHIVertexFormat format, byte stream,
		                                     uint16_t offset) override;
		VulkanContext& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream,
		                                  RHIVertexInputRate rate) override;
		VulkanContext& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format, size_t byte_offset = 0) override;
		VulkanContext& bind_uniform_buffer(RHIBuffer* buffer, byte slot) override;
		VulkanContext& bind_uniform_buffer(VulkanBuffer* buffer, uint32_t size, uint32_t offset, byte slot);
		VulkanContext& bind_srv(RHIShaderResourceView* view, byte slot) override;
		VulkanContext& bind_uav(RHIUnorderedAccessView* view, byte slot) override;
		VulkanContext& bind_sampler(RHISampler* view, byte slot) override;
		VulkanContext& bind_pipeline(RHIPipeline* pipeline) override;
		VulkanContext& bind_acceleration(RHIAccelerationStructure* acceleration, byte slot) override;

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
}// namespace Engine
