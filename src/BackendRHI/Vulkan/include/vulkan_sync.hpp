#pragma once
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanCommandHandle;

	class VulkanFence : public VulkanDeferredDestroy<RHIFence>
	{
	private:
		enum class Status
		{
			Undefined  = -1,
			Unsignaled = 0,
			Signaled   = 1,
		};

		mutable Status m_status;
		vk::Fence m_fence;

		bool update_status() const;

	public:
		VulkanFence(bool is_signaled = false);
		~VulkanFence();

		bool is_signaled() override;
		void reset() override;

		VulkanFence& wait();
		inline vk::Fence fence() const { return m_fence; }

		inline VulkanFence& make_pending()
		{
			if (m_status == Status::Signaled)
				reset();

			m_status = Status::Undefined;
			return *this;
		}
	};

	class VulkanSemaphore : public VulkanDeferredDestroy<RHISemaphore>
	{
	private:
		vk::Semaphore m_semaphore;
		bool m_signaled;

	public:
		VulkanSemaphore();
		~VulkanSemaphore();

		inline const vk::Semaphore& semaphore() const { return m_semaphore; }
		inline bool is_signaled() const { return m_signaled; }
		inline VulkanSemaphore& is_signaled(bool signaled) { trinex_this_return(m_signaled = signaled); }
	};
}// namespace Trinex
