#pragma once
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	class VulkanCommandBuffer;
	class VulkanTexture;

	class VulkanSwapchain : public VulkanDeferredDestroy<RHISwapchain>
	{
	private:
		enum Status : int_t
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
		Vector<Semaphore> m_image_present_semaphores;
		Vector<Semaphore> m_render_finished_semaphores;

		vk::SurfaceKHR m_surface;
		vk::PresentModeKHR m_present_mode;
		vk::SwapchainKHR m_swapchain;
		int32_t m_sync_index  = 0;
		int32_t m_image_index = -1;
		bool m_need_recreate  = false;

	private:
		VulkanSwapchain& create_swapchain(vk::SwapchainKHR* old = nullptr);
		VulkanSwapchain& release_swapchain();
		VulkanSwapchain& recreate_swapchain();

	public:
		VulkanSwapchain(Window* window, bool vsync);
		~VulkanSwapchain();

		int_t acquire_image_index(VulkanCommandBuffer* cmd_buffer);
		int_t do_present(VulkanCommandBuffer* cmd_buffer);
		VulkanTexture* backbuffer();
		int_t try_present(int_t (VulkanSwapchain::*callback)(VulkanCommandBuffer*), VulkanCommandBuffer* cmd_buffer,
		                  bool skip_on_out_of_date);

		vk::Semaphore render_finished_semaphore();
		vk::Semaphore image_present_semaphore();

		void present();
		void resize(const Vector2u& size) override;
		void vsync(bool flag) override;
		RHI_RenderTargetView* as_rtv() override;
	};
}// namespace Engine
