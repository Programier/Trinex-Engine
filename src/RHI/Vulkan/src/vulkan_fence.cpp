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

	VulkanFence& VulkanFence::reset()
	{
		API->m_device.resetFences(m_fence);
		m_is_signaled = false;
		return *this;
	}

	VulkanFence* VulkanFence::create(bool is_signaled)
	{
		return new VulkanFence(is_signaled);
	}

	void VulkanFence::release(VulkanFence* fence)
	{
		delete fence;
	}


	VulkanFence::~VulkanFence()
	{
		DESTROY_CALL(destroyFence, m_fence);
	}
}// namespace Engine
