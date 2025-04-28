#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
	struct VulkanFence : public RHI_DefaultDestroyable<RHI_Fence> {
	private:
		mutable bool m_is_signaled;

		VulkanFence(bool is_signaled);
		bool update_status() const;
		~VulkanFence();

	public:
		vk::Fence m_fence;

		inline bool is_signaled() override { return m_is_signaled || update_status(); }
		void reset() override;
		void wait() override;

		static VulkanFence* create(bool is_signaled);
	};
}// namespace Engine
