#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/critical_section.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanCommandHandle;

	class VulkanQueue
	{
	private:
		CriticalSection m_critical;
		vk::Queue m_queue;
		u32 m_index;

	public:
		VulkanQueue(vk::Queue queue, u32 index);
		VulkanQueue& submit(const vk::SubmitInfo& info, vk::Fence fence = {});
		vk::Result present(const vk::PresentInfoKHR& info);
		VulkanQueue& idle();
		inline u32 index() const { return m_index; }
	};
}// namespace Trinex
