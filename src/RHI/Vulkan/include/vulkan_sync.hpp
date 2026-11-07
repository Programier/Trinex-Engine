#pragma once
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanFence final : public VulkanDeferredDestroy<RHIFence>
	{
	private:
		vk::Semaphore m_semaphore;
		vk::SemaphoreType m_type;
		bool m_signaled;

	public:
		VulkanFence(vk::SemaphoreType type, u64 value = 0);
		~VulkanFence();

		RHI* device() const override;
		
		u64 value() const override;
		bool wait(u64 value, u64 timeout = ~0ull) const override;
		bool signal(u64 value) override;

		inline const vk::Semaphore& semaphore() const { return m_semaphore; }
		inline bool is_timeline() const { return m_type == vk::SemaphoreType::eTimeline; }
		inline bool is_binary() const { return m_type == vk::SemaphoreType::eBinary; }
		inline bool is_signaled() const { return m_signaled; }
		inline VulkanFence& is_signaled(bool value) { trinex_this_return(m_signaled = value); }
	};
} // namespace Trinex
