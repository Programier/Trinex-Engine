#pragma once
#include <Core/etl/array.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	class VulkanQueue;
	class VulkanTexture;

	class VulkanSwapchain final : public RHISwapchain
	{
	private:
		Array<VulkanTexture*> m_backbuffers;
		Array<RHIResourcePtr<VulkanFence>> m_image_present_fences;
		Array<RHIResourcePtr<VulkanFence>> m_render_finished_fences;

		vk::SurfaceKHR m_surface;
		Window* m_window;
		vk::PresentModeKHR m_present_mode;
		vk::SwapchainKHR m_swapchain;
		u32 m_desired_image_count;
		i32 m_sync_index  = 0;
		i32 m_image_index = -1;
		Vector2u m_size;
		bool m_need_recreate = false;

	private:
		VulkanSwapchain& create_swapchain(vk::SwapchainKHR* old = nullptr);
		VulkanSwapchain& release_swapchain();
		VulkanSwapchain& recreate_swapchain();
		VulkanSwapchain& recreate_surface();

	public:
		VulkanSwapchain(Window* window, RHIPresentMode mode, u32 images);
		~VulkanSwapchain();

		RHI* device() const override;
		
		i32 acquire_image_index();
		VulkanFence* acquire_fence() override;
		VulkanFence* present_fence() override;

		vk::Result present(vk::Queue* queue);
		void resize(Vector2u size) override;
		void mode(RHIPresentMode mode) override;
		RHITexture* backbuffer() override;
		void destroy() override;
	};
} // namespace Trinex
