#pragma once
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	class VulkanCommandHandle;
	class VulkanTexture;

	class VulkanSwapchain : public RHISwapchain
	{
	private:
		enum Status : i32
		{
			Success     = 0,
			OutOfDate   = -1,
			SurfaceLost = -2,
		};

		class Semaphore
		{
		private:
			vk::Semaphore m_semaphore;

		public:
			Semaphore();
			Semaphore(const Semaphore& semaphore) = delete;
			Semaphore(Semaphore&& semaphore);
			~Semaphore();

			inline vk::Semaphore semaphore() const { return m_semaphore; }
		};

		Vector<VulkanTexture*> m_backbuffers;
		VulkanTexture* m_render_buffer = nullptr;
		Vector<Semaphore> m_image_present_semaphores;
		Vector<Semaphore> m_render_finished_semaphores;

		vk::SurfaceKHR m_surface;
		vk::PresentModeKHR m_present_mode;
		vk::SwapchainKHR m_swapchain;
		i32 m_sync_index  = 0;
		i32 m_image_index = -1;
		Vector2u m_size;
		bool m_need_recreate  = false;

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

		vk::Semaphore render_finished_semaphore();
		vk::Semaphore image_present_semaphore();

		void present();
		void resize(const Vector2u& size) override;
		void vsync(bool flag) override;
		RHIRenderTargetView* as_rtv() override;
		RHITexture* as_texture() override;
		void destroy() override;
	};
}// namespace Engine
