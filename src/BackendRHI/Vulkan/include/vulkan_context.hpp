#pragma once
#include <Core/etl/vector.hpp>
#include <RHI/context.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
	class RHIObject;
	class VulkanRenderTarget;

	class VulkanCommandHandle final : public vk::CommandBuffer, public RHICommandHandle
	{
	private:
		enum class State
		{
			IsReadyForBegin,
			IsInsideBegin,
			IsInsideRenderPass,
			HasEnded,
			Submitted,
		};

		Vector<vk::Semaphore> m_wait_semaphores;
		Vector<vk::PipelineStageFlags> m_wait_flags;
		class VulkanFence* m_fence    = nullptr;
		size_t m_fence_signaled_count = 0;

		State m_state = State::IsReadyForBegin;

	public:
		VulkanCommandHandle(class VulkanCommandBufferManager* manager);
		VulkanCommandHandle(const VulkanCommandHandle&) = delete;
		VulkanCommandHandle(VulkanCommandHandle&&)      = delete;
		~VulkanCommandHandle();

		VulkanCommandHandle& operator=(const VulkanCommandHandle&) = delete;
		VulkanCommandHandle& operator=(VulkanCommandHandle&&)      = delete;

		VulkanCommandHandle& refresh_fence_status();
		VulkanCommandHandle& begin();
		VulkanCommandHandle& end();
		VulkanCommandHandle& begin_render_pass(VulkanRenderTarget* rt);
		VulkanCommandHandle& end_render_pass();
		VulkanCommandHandle& add_wait_semaphore(vk::PipelineStageFlags flags, vk::Semaphore semaphore);
		VulkanCommandHandle& submit(vk::Semaphore semaphore = VK_NULL_HANDLE);
		VulkanCommandHandle& wait();

		void destroy() override {}

		inline bool is_ready_for_begin() const { return m_state == State::IsReadyForBegin; }
		inline bool is_inside_render_pass() const { return m_state == State::IsInsideRenderPass; }
		inline bool is_outside_render_pass() const { return m_state == State::IsInsideBegin; }
		inline bool has_begun() const { return m_state == State::IsInsideBegin || m_state == State::IsInsideRenderPass; }
		inline bool has_ended() const { return m_state == State::HasEnded; }
		inline bool is_submitted() const { return m_state == State::Submitted; }
		inline size_t fence_signaled_count() const { return m_fence_signaled_count; }
		inline VulkanFence* fence() const { return m_fence; }
		inline const Vector<vk::Semaphore>& wait_semaphores() const { return m_wait_semaphores; }
		inline const Vector<vk::PipelineStageFlags>& wait_flags() const { return m_wait_flags; }
	};

	class VulkanCommandBufferManager
	{
	private:
		struct Node {
			VulkanCommandHandle* command_buffer = nullptr;
			Node* next                          = nullptr;
		};

		vk::CommandPool m_pool;
		Node* m_first     = nullptr;
		Node** m_push_ptr = nullptr;
		Node* m_current   = nullptr;

		Node* create_node();
		Node* destroy_node(Node* node);
		Node* request_node();

	public:
		VulkanCommandBufferManager();
		~VulkanCommandBufferManager();
		FORCE_INLINE VulkanCommandHandle* current() const { return m_current->command_buffer; }
		VulkanCommandBufferManager& submit(vk::Semaphore semaphore = VK_NULL_HANDLE);
		inline vk::CommandPool command_pool() const { return m_pool; }
	};

	class VulkanContext : public RHIContext
	{
	private:
		VulkanCommandHandle* m_cmd = nullptr;

	public:
		VulkanContext& begin() override;
		VulkanCommandHandle* end() override;

		VulkanContext& execute(RHICommandHandle* handle) override;

		VulkanContext& draw(size_t vertex_count, size_t vertices_offset) override;
		VulkanContext& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
		VulkanContext& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances) override;
		VulkanContext& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                      size_t instances) override;

		VulkanContext& draw_mesh(uint32_t x, uint32_t y, uint32_t z) override;
		VulkanContext& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
	};
}// namespace Engine
