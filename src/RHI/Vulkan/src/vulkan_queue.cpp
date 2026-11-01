#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_swapchain.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	VulkanQueue::VulkanQueue(vk::Queue queue, u32 index) : m_queue(queue), m_index(index) {}

	void VulkanQueue::destroy()
	{
		trx_delete this;
	}

	RHIQueueFlags VulkanQueue::flags() const
	{
		return {};
	}

	void VulkanQueue::present(RHISwapchain* swapchain)
	{
		trinex_assert(swapchain);
		static_cast<VulkanSwapchain*>(swapchain)->present(&m_queue);
	}

	void VulkanQueue::submit(const RHISubmitInfo& info)
	{
		vk::SubmitInfo submit;

		if (info.command)
		{
			vk::CommandBuffer& cmd = *static_cast<VulkanCommandHandle*>(info.command);
			submit.setCommandBuffers(cmd);
		}

		if (info.wait)
		{
			VulkanSemaphore* semaphore = static_cast<VulkanSemaphore*>(info.wait);
			trinex_assert(semaphore->is_signaled());
			submit.setWaitSemaphores(semaphore->is_signaled(false).semaphore());

			vk::PipelineStageFlags wait_mask = vk::PipelineStageFlagBits::eAllCommands;
			submit.setWaitDstStageMask(wait_mask);
		}

		if (info.signal)
		{
			VulkanSemaphore* semaphore = static_cast<VulkanSemaphore*>(info.signal);
			trinex_assert(!semaphore->is_signaled());
			submit.setSignalSemaphores(semaphore->is_signaled(true).semaphore());
		}

		VulkanFence* signal = static_cast<VulkanFence*>(info.fence);
		vk::Fence fence     = info.fence ? signal->make_pending().fence() : VK_NULL_HANDLE;

		vk::verify(m_queue.submit(submit, fence));
	}

	void VulkanQueue::wait()
	{
		vk::verify(m_queue.waitIdle());
	}

	VulkanQueue& VulkanQueue::submit(const vk::SubmitInfo& info, vk::Fence fence)
	{
		ScopeLock lock(m_critical);
		vk::verify(m_queue.submit(info, fence));
		return *this;
	}
} // namespace Trinex
