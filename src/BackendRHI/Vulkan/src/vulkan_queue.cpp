#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_queue.hpp>


namespace Engine
{
	VulkanQueue::VulkanQueue(vk::Queue queue, uint32_t index) : m_queue(queue), m_index(index) {}

	VulkanQueue& VulkanQueue::submit(VulkanCommandHandle* cmd_buffer, uint32_t signal_semaphores_count,
	                                 const vk::Semaphore* signal_semaphores)
	{
		auto& wait_semaphores = cmd_buffer->wait_semaphores();
		auto& wait_flags      = cmd_buffer->wait_flags();

		vk::SubmitInfo info(wait_semaphores.size(), wait_semaphores.data(), wait_flags.data(), 1, cmd_buffer,
		                    signal_semaphores_count, signal_semaphores);
		m_queue.submit(info, cmd_buffer->fence()->fence());
		return *this;
	}

	VulkanQueue& VulkanQueue::submit(const vk::SubmitInfo& info, vk::Fence fence)
	{
		m_queue.submit(info, fence);
		return *this;
	}
}// namespace Engine
