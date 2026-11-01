#pragma once
#include <Core/etl/critical_section.hpp>
#include <RHI/handles.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanCommandHandle;

	class VulkanQueue final : public RHIQueue
	{
	private:
		vk::Queue m_queue;
		RHIQueueFlags m_flags;
		u32 m_family_index;
		u32 m_queue_index;

	public:
		VulkanQueue(vk::Queue queue, RHIQueueFlags flags, u32 family_index, u32 queue_index);

		void destroy() override;
		RHIQueueFlags flags() const override;
		void present(RHISwapchain* swapchain) override;
		void submit(const RHISubmitInfo& info) override;
		void wait() override;

		inline u32 family_index() const { return m_family_index; }
		inline u32 queue_index() const { return m_queue_index; }
	};
} // namespace Trinex
