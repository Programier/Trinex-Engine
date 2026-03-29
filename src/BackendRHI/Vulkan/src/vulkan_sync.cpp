#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	VulkanFence::VulkanFence(bool is_signaled)
	{
		if (is_signaled)
		{
			m_fence = vk::check_result(
			        VulkanAPI::instance()->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
			m_status = Status::Signaled;
		}
		else
		{
			m_fence  = vk::check_result(VulkanAPI::instance()->m_device.createFence({}));
			m_status = Status::Unsignaled;
		}
	}

	bool VulkanFence::update_status() const
	{
		if (VulkanAPI::instance()->m_device.getFenceStatus(m_fence) == vk::Result::eSuccess)
		{
			m_status = Status::Signaled;
			return true;
		}
		return false;
	}

	bool VulkanFence::is_signaled()
	{
		if (m_status == Status::Undefined)
		{
			return update_status();
		}

		return m_status == Status::Signaled;
	}

	void VulkanFence::reset()
	{
		vk::check_result(VulkanAPI::instance()->m_device.resetFences(m_fence));
		m_status = Status::Unsignaled;
	}

	VulkanFence& VulkanFence::wait()
	{
		auto result = VulkanAPI::instance()->m_device.waitForFences(m_fence, vk::True, UINT64_MAX);
		(void) result;
		return *this;
	}

	VulkanFence::~VulkanFence()
	{
		DESTROY_CALL(destroyFence, m_fence);
	}

	VulkanSemaphore::VulkanSemaphore() : m_signaled(false)
	{
		m_semaphore = vk::check_result(VulkanAPI::instance()->m_device.createSemaphore(vk::SemaphoreCreateInfo()));
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		VulkanAPI::instance()->m_device.destroySemaphore(m_semaphore);
	}

	RHIFence* VulkanAPI::create_fence()
	{
		return trx_new VulkanFence();
	}
}// namespace Trinex
