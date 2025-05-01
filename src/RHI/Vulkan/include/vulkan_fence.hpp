#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanCommandBuffer;

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

	struct VulkanFenceRef : public RHI_DefaultDestroyable<RHI_Fence> {
		VulkanCommandBuffer* m_cmd    = nullptr;
		size_t m_fence_signaled_count = 0;

		bool is_signaled() override;
		void reset() override;
	};
}// namespace Engine
