#pragma once
#include <Core/engine_types.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanQueue {
		vk::Queue m_queue;
		uint32_t m_index;

		struct VulkanCommandBuffer* m_last_submitted_cmd_buffer;

	public:
		VulkanQueue(vk::Queue queue, uint32_t index);

		VulkanQueue& submit(VulkanCommandBuffer* cmd_buffer, uint32_t signal_semaphores_count = 0,
		                    const vk::Semaphore* signal_semaphores = nullptr);

		inline VulkanQueue& submit(VulkanCommandBuffer* cmd_buffer, vk::Semaphore signal_semaphore)
		{
			return submit(cmd_buffer, 1, &signal_semaphore);
		}

		VulkanQueue& submit(const vk::SubmitInfo& info, vk::Fence fence = {});
		VulkanQueue& wait_idle();
	};
}// namespace Engine
