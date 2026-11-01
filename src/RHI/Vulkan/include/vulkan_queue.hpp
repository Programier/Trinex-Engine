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
		u32 m_index;

	public:
		VulkanQueue(vk::Queue queue, u32 index);

		void destroy() override;
		RHIQueueFlags flags() const override;
		void present(RHISwapchain* swapchain) override;
		void submit(const RHISubmitInfo& info) override;
		void wait() override;

		inline u32 index() const { return m_index; }
	};
} // namespace Trinex
