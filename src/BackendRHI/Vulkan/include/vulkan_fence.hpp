#pragma once
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanCommandBuffer;

	class VulkanFence
	{
	private:
		mutable bool m_is_signaled;
		vk::Fence m_fence;

		bool update_status() const;

	public:
		VulkanFence(bool is_signaled);
		~VulkanFence();

		bool is_signaled() const { return m_is_signaled || update_status(); }
		VulkanFence& reset();
		VulkanFence& wait();

		inline vk::Fence fence() const { return m_fence; }
	};

	class VulkanFenceRef : public VulkanDeferredDestroy<RHI_Fence>
	{
	private:
		VulkanCommandBuffer* m_cmd    = nullptr;
		size_t m_fence_signaled_count = 0;

	public:
		bool is_signaled() override;
		void reset() override;
		VulkanFenceRef& signal(VulkanCommandBuffer* cmd_buffer);
		inline bool is_waiting() const { return m_cmd != nullptr; }
	};
}// namespace Engine
