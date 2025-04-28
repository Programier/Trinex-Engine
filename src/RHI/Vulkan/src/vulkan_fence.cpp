#include <vulkan_api.hpp>
#include <vulkan_fence.hpp>

namespace Engine
{
	VulkanFence::VulkanFence(bool is_signaled) : m_is_signaled(is_signaled)
	{
		if (is_signaled)
			m_fence = API->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		else
			m_fence = API->m_device.createFence({});
	}

	bool VulkanFence::update_status() const
	{
		m_is_signaled = API->m_device.getFenceStatus(m_fence) == vk::Result::eSuccess;
		return m_is_signaled;
	}

	void VulkanFence::reset()
	{
		API->m_device.resetFences(m_fence);
		m_is_signaled = false;
	}

	void VulkanFence::wait()
	{
		auto result = API->m_device.waitForFences(m_fence, vk::True, UINT64_MAX);
		(void) result;
	}

	VulkanFence* VulkanFence::create(bool is_signaled)
	{
		return new VulkanFence(is_signaled);
	}

	VulkanFence::~VulkanFence()
	{
		DESTROY_CALL(destroyFence, m_fence);
	}

	RHI_Fence* VulkanAPI::create_fence()
	{
		return VulkanFence::create(false);
	}
}// namespace Engine
