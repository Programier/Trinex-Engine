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

		VulkanQueue& submit(VulkanCommandHandle* cmd_buffer, uint32_t signal_semaphores_count = 0,
		                    const vk::Semaphore* signal_semaphores = nullptr);

		inline VulkanQueue& submit(VulkanCommandHandle* cmd_buffer, vk::Semaphore signal_semaphore)
		{
			return submit(cmd_buffer, 1, &signal_semaphore);
		}

		VulkanQueue& submit(const vk::SubmitInfo& info, vk::Fence fence = {});
		inline vk::Queue queue() const { return m_queue; }
		inline uint32_t index() const { return m_index; }
	};
}// namespace Engine
