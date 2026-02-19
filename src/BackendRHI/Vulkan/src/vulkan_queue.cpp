#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_queue.hpp>


namespace Engine
{
	VulkanQueue::VulkanQueue(vk::Queue queue, uint32_t index) : m_queue(queue), m_index(index) {}

	VulkanQueue& VulkanQueue::submit(const vk::SubmitInfo& info, vk::Fence fence)
	{
		ScopeLock lock(m_critical);
		vk::check_result(m_queue.submit(info, fence));
		return *this;
	}

	vk::Result VulkanQueue::present(const vk::PresentInfoKHR& info)
	{
		ScopeLock lock(m_critical);
		vk::Result result = m_queue.presentKHR(info);
		return result;
	}

	VulkanQueue& VulkanQueue::idle()
	{
		ScopeLock lock(m_critical);
		vk::check_result(m_queue.waitIdle());
		return *this;
	}
}// namespace Engine
