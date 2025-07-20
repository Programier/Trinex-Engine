#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
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

	bool VulkanFenceRef::is_signaled()
	{
		if (m_cmd)
		{
			m_cmd->refresh_fence_status();
			return m_fence_signaled_count != m_cmd->fence_signaled_count();
		}
		return false;
	}

	void VulkanFenceRef::reset()
	{
		m_cmd = nullptr;
	}

	VulkanFenceRef& VulkanFenceRef::signal(VulkanCommandBuffer* cmd_buffer)
	{
		m_cmd                  = cmd_buffer;
		m_fence_signaled_count = cmd_buffer->fence_signaled_count();
		return *this;
	}

	RHIFence* VulkanAPI::create_fence()
	{
		return new VulkanFenceRef();
	}

	VulkanAPI& VulkanAPI::signal_fence(RHIFence* fence)
	{
		static_cast<VulkanFenceRef*>(fence)->signal(current_command_buffer());
		return *this;
	}
}// namespace Engine
