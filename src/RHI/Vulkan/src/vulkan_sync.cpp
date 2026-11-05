#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	VulkanFence::VulkanFence(vk::SemaphoreType type, u64 value) : m_type(type), m_signaled(false)
	{
		vk::SemaphoreTypeCreateInfo type_info = {type, value};
		m_semaphore = vk::verify(VulkanAPI::device()->createSemaphore(vk::SemaphoreCreateInfo({}, type_info)));
	}

	VulkanFence::~VulkanFence()
	{
		VulkanAPI::device()->destroySemaphore(m_semaphore);
	}

	u64 VulkanFence::value() const
	{
		if (m_type == vk::SemaphoreType::eTimeline)
			return vk::verify(VulkanAPI::device()->getSemaphoreCounterValue(m_semaphore));

		trinex_error(Log::RHI, "Counter values are only supported for timeline semaphores");
		return 0;
	}

	bool VulkanFence::wait(u64 value, u64 timeout) const
	{
		if (m_type == vk::SemaphoreType::eTimeline)
		{
			vk::SemaphoreWaitInfo info;
			info.setSemaphores(m_semaphore);
			info.setValues(value);

			vk::Result result = VulkanAPI::device()->waitSemaphores(info, timeout);
			return result == vk::Result::eSuccess;
		}

		trinex_error(Log::RHI,
		             "CPU wait is only supported for timeline semaphores; use an RHIFence for binary semaphore completion");
		return false;
	}

	bool VulkanFence::signal(u64 value)
	{
		if (m_type == vk::SemaphoreType::eTimeline)
		{
			vk::SemaphoreSignalInfo info(m_semaphore, value);
			auto result = VulkanAPI::device()->signalSemaphore(info);
			return result == vk::Result::eSuccess;
		}

		trinex_error(Log::RHI, "CPU signal is only supported for timeline semaphores");
		return false;
	}

	RHIFence* VulkanAPI::create_fence(u64 value)
	{
		return trx_new VulkanFence(vk::SemaphoreType::eTimeline, value);
	}
} // namespace Trinex
