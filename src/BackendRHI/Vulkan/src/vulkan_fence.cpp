#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_queue.hpp>

namespace Engine
{
	VulkanFence::VulkanFence(bool is_signaled)
	{
		if (is_signaled)
		{
			m_fence  = vk::check_result(API->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
			m_status = Status::Signaled;
		}
		else
		{
			m_fence  = vk::check_result(API->m_device.createFence({}));
			m_status = Status::Unsignaled;
		}
	}

	bool VulkanFence::update_status() const
	{
		if (API->m_device.getFenceStatus(m_fence) == vk::Result::eSuccess)
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
		vk::check_result(API->m_device.resetFences(m_fence));
		m_status = Status::Unsignaled;
	}

	VulkanFence& VulkanFence::wait()
	{
		auto result = API->m_device.waitForFences(m_fence, vk::True, UINT64_MAX);
		(void) result;
		return *this;
	}

	VulkanFence::~VulkanFence()
	{
		DESTROY_CALL(destroyFence, m_fence);
	}

	RHIFence* VulkanAPI::create_fence()
	{
		return trx_new VulkanFence();
	}

	VulkanAPI& VulkanAPI::signal(RHIFence* fence)
	{
		static_cast<VulkanFence*>(fence)->make_pending();
		m_graphics_queue->submit({}, static_cast<VulkanFence*>(fence)->fence());
		return *this;
	}
}// namespace Engine
