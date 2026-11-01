#include <Core/etl/allocator.hpp>
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
		StackByteAllocator::Mark mark;

		vk::SubmitInfo submit;
		Array<vk::CommandBuffer, StackAllocator<vk::CommandBuffer>> commands;
		Array<vk::Semaphore, StackAllocator<vk::Semaphore>> waits;
		Array<vk::PipelineStageFlags, StackAllocator<vk::PipelineStageFlags>> wait_stages;
		Array<vk::Semaphore, StackAllocator<vk::Semaphore>> signals;

		for (RHICommandHandle* command : info.commands)
		{
			commands.push_back(*static_cast<VulkanCommandHandle*>(command));
		}
		submit.setCommandBuffers(commands);

		for (RHISemaphore* wait : info.waits)
		{
			VulkanSemaphore* semaphore = static_cast<VulkanSemaphore*>(wait);
			trinex_assert(semaphore->is_signaled());
			waits.push_back(semaphore->is_signaled(false).semaphore());
			wait_stages.push_back(vk::PipelineStageFlagBits::eAllCommands);
		}
		submit.setWaitSemaphores(waits).setWaitDstStageMask(wait_stages);

		for (RHISemaphore* signal : info.signals)
		{
			VulkanSemaphore* semaphore = static_cast<VulkanSemaphore*>(signal);
			trinex_assert(!semaphore->is_signaled());
			signals.push_back(semaphore->is_signaled(true).semaphore());
		}
		submit.setSignalSemaphores(signals);

		VulkanFence* signal = static_cast<VulkanFence*>(info.fence);
		vk::Fence fence     = info.fence ? signal->make_pending().fence() : VK_NULL_HANDLE;

		vk::verify(m_queue.submit(submit, fence));
	}

	void VulkanQueue::wait()
	{
		vk::verify(m_queue.waitIdle());
	}
} // namespace Trinex
