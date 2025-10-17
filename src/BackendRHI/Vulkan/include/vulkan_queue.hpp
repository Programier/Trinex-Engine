#pragma once
#include <Core/engine_types.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanCommandHandle;

	class VulkanQueue
	{
		vk::Queue m_queue;
		uint32_t m_index;

	public:
		VulkanQueue(vk::Queue queue, uint32_t index);
		VulkanQueue& submit(const vk::SubmitInfo& info, vk::Fence fence = {});
		inline vk::Queue queue() const { return m_queue; }
		inline uint32_t index() const { return m_index; }
	};
}// namespace Engine
