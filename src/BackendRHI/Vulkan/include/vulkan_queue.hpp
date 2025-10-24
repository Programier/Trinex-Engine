#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/critical_section.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanCommandHandle;

	class VulkanQueue
	{
	private:
		CriticalSection m_critical;
		vk::Queue m_queue;
		uint32_t m_index;

	public:
		VulkanQueue(vk::Queue queue, uint32_t index);
		VulkanQueue& submit(const vk::SubmitInfo& info, vk::Fence fence = {});
		vk::Result present(const vk::PresentInfoKHR& info);
		VulkanQueue& idle();
		inline uint32_t index() const { return m_index; }
	};
}// namespace Engine
