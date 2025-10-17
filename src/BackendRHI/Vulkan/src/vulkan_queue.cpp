#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_queue.hpp>


namespace Engine
{
	VulkanQueue::VulkanQueue(vk::Queue queue, uint32_t index) : m_queue(queue), m_index(index) {}

	VulkanQueue& VulkanQueue::submit(const vk::SubmitInfo& info, vk::Fence fence)
	{
		m_queue.submit(info, fence);
		return *this;
	}
}// namespace Engine
