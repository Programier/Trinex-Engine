#pragma once
#include <Core/etl/vector.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	class VulkanCommandHandle;
	class VulkanTexture;

	class VulkanSwapchain final : public RHISwapchain
	{
	private:
		enum Status : i32
		{
			Success     = 0,
			OutOfDate   = -1,
			SurfaceLost = -2,
		};

		Vector<VulkanTexture*> m_backbuffers;
		Vector<RHIResourcePtr<VulkanSemaphore>> m_image_present_semaphores;
		Vector<RHIResourcePtr<VulkanSemaphore>> m_render_finished_semaphores;

		vk::SurfaceKHR m_surface;
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

	public:
		VulkanSwapchain(Window* window, bool vsync);
		~VulkanSwapchain();

		i32 acquire_image_index();
		i32 do_present();
		VulkanTexture* backbuffer();
		i32 try_present(i32 (VulkanSwapchain::*callback)(), bool skip_on_out_of_date);

		VulkanSemaphore* acquire_semaphore() override;
		VulkanSemaphore* present_semaphore() override;

		void present();
		void resize(const Vector2u& size) override;
		void vsync(bool flag) override;
		RHIRenderTargetView* as_rtv() override;
		RHITexture* as_texture() override;
		void destroy() override;
	};
}// namespace Trinex
