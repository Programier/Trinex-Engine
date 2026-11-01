#pragma once
#include <Core/etl/array.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>
#include <VkBootstrap.h>
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
		Array<RHIResourcePtr<VulkanSemaphore>> m_image_present_semaphores;
		Array<RHIResourcePtr<VulkanSemaphore>> m_render_finished_semaphores;

		vk::SurfaceKHR m_surface;
		Window* m_window;
		vk::PresentModeKHR m_present_mode;
		vk::SwapchainKHR m_swapchain;
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

		i32 acquire_image_index();
		VulkanSemaphore* acquire_semaphore() override;
		VulkanSemaphore* present_semaphore() override;

		vk::Result present(vk::Queue* queue);
		void resize(Vector2u size) override;
		void mode(RHIPresentMode mode) override;
		RHITexture* backbuffer() override;
		void destroy() override;
	};
} // namespace Trinex
