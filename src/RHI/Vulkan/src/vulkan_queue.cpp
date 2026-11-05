#include <Core/etl/allocator.hpp>
#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_swapchain.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	VulkanQueue::VulkanQueue(vk::Queue queue, RHIQueueFlags flags, u32 family_index, u32 queue_index)
	    : m_queue(queue), m_submit_index(0), m_family_index(family_index), m_queue_index(queue_index), m_flags(flags)
	{
		m_submit_fence = trx_new VulkanFence(vk::SemaphoreType::eTimeline, 0);
	}

	VulkanQueue::~VulkanQueue()
	{
		m_submit_fence->release();
	}

	void VulkanQueue::destroy()
	{
		trx_delete this;
	}

	RHIQueueFlags VulkanQueue::flags() const
	{
		return m_flags;
	}

	RHIFence* VulkanQueue::fence() const
	{
		return m_submit_fence;
	}

	u64 VulkanQueue::last_submitted() const
	{
		return m_submit_index;
	}

	void VulkanQueue::present(RHISwapchain* swapchain)
	{
		trinex_assert(swapchain);
		static_cast<VulkanSwapchain*>(swapchain)->present(&m_queue);
	}

	RHIFencePoint VulkanQueue::submit(const RHISubmitInfo& info)
	{
		StackByteAllocator::Mark mark;

		vk::SubmitInfo submit;
		Array<vk::CommandBuffer, StackAllocator<vk::CommandBuffer>> commands;
		Array<vk::Semaphore, StackAllocator<vk::Semaphore>> waits;
		Array<vk::PipelineStageFlags, StackAllocator<vk::PipelineStageFlags>> wait_stages;
		Array<u64, StackAllocator<u64>> wait_values;
		Array<vk::Semaphore, StackAllocator<vk::Semaphore>> signals;
		Array<u64, StackAllocator<u64>> signal_values;

		commands.reserve(info.commands.size());
		waits.reserve(info.waits.size());
		wait_stages.reserve(info.waits.size());
		wait_values.reserve(info.waits.size());
		signals.reserve(info.signals.size() + 1);
		signal_values.reserve(info.signals.size() + 1);

		for (RHICommandHandle* command : info.commands)
		{
			commands.push_back(*static_cast<VulkanCommandHandle*>(command));
		}
		submit.setCommandBuffers(commands);

		for (const RHIFencePoint& point : info.waits)
		{
			trinex_assert(point.fence);
			VulkanFence* fence = static_cast<VulkanFence*>(point.fence);
			if (!fence->is_timeline())
			{
				trinex_assert(point.value == 0);
				trinex_assert(fence->is_signaled());
				fence->is_signaled(false);
			}

			waits.push_back(fence->semaphore());
			wait_stages.push_back(vk::PipelineStageFlagBits::eAllCommands);
			wait_values.push_back(point.value);
		}
		submit.setWaitSemaphores(waits).setWaitDstStageMask(wait_stages);

		for (const RHIFencePoint& point : info.signals)
		{
			trinex_assert(point.fence);
			VulkanFence* fence = static_cast<VulkanFence*>(point.fence);

			if (!fence->is_timeline())
			{
				trinex_assert(point.value == 0);
				trinex_assert(!fence->is_signaled());
				fence->is_signaled(true);
			}

			signals.push_back(fence->semaphore());
			signal_values.push_back(point.value);
		}

		signals.push_back(m_submit_fence->semaphore());
		signal_values.push_back(++m_submit_index);
		submit.setSignalSemaphores(signals);

		vk::TimelineSemaphoreSubmitInfo timeline;
		timeline.waitSemaphoreValueCount   = wait_values.size();
		timeline.pWaitSemaphoreValues      = wait_values.data();
		timeline.signalSemaphoreValueCount = signal_values.size();
		timeline.pSignalSemaphoreValues    = signal_values.data();
		submit.pNext                       = &timeline;

		vk::verify(m_queue.submit(submit));
		return {m_submit_fence, m_submit_index};
	}

	void VulkanQueue::wait()
	{
		vk::verify(m_queue.waitIdle());
	}
} // namespace Trinex
