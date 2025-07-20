#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
	class RHIObject;
	class VulkanRenderTarget;

	class VulkanCommandBuffer final : public vk::CommandBuffer
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

		Vector<RHIObject*> m_pending_destroy;
		Vector<vk::Semaphore> m_wait_semaphores;
		Vector<vk::PipelineStageFlags> m_wait_flags;
		class VulkanFence* m_fence    = nullptr;
		size_t m_fence_signaled_count = 0;

		State m_state = State::IsReadyForBegin;

	private:
		VulkanCommandBuffer& destroy_objects();

	public:
		VulkanCommandBuffer(class VulkanCommandBufferManager* manager);
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&)      = delete;
		~VulkanCommandBuffer();

		VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&)      = delete;

		VulkanCommandBuffer& refresh_fence_status();
		VulkanCommandBuffer& begin();
		VulkanCommandBuffer& end();
		VulkanCommandBuffer& begin_render_pass(VulkanRenderTarget* rt);
		VulkanCommandBuffer& end_render_pass();
		VulkanCommandBuffer& add_wait_semaphore(vk::PipelineStageFlags flags, vk::Semaphore semaphore);
		VulkanCommandBuffer& submit(vk::Semaphore semaphore = VK_NULL_HANDLE);
		VulkanCommandBuffer& wait();
		VulkanCommandBuffer& destroy_object(RHIObject* object);

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
			VulkanCommandBuffer* command_buffer = nullptr;
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
		FORCE_INLINE VulkanCommandBuffer* current() const { return m_current->command_buffer; }
		VulkanCommandBufferManager& submit(vk::Semaphore semaphore = VK_NULL_HANDLE);
		inline vk::CommandPool command_pool() const { return m_pool; }
	};
}// namespace Engine
