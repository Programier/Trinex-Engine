#pragma once
#include <vulkan_headers.hpp>


namespace Engine
{
	struct VulkanFence {
	private:
		mutable bool m_is_signaled;

		VulkanFence(bool is_signaled);
		bool update_status() const;
		~VulkanFence();

	public:
		vk::Fence m_fence;

		bool is_signaled() const { return m_is_signaled || update_status(); }
		VulkanFence& reset();
		VulkanFence& wait();

		static VulkanFence* create(bool is_signaled);
		static void release(VulkanFence* fence);
	};
}// namespace Engine
