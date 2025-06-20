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
	struct VulkanSwapchainRenderTarget;

	struct VulkanSwapchain {
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

		Vector<VulkanSwapchainRenderTarget*> m_backbuffers;
		Vector<Semaphore> m_image_present_semaphores;
		Vector<Semaphore> m_render_finished_semaphores;

		vk::SurfaceKHR m_surface;
		vk::PresentModeKHR m_present_mode;
		vk::SwapchainKHR m_swapchain;
		int32_t m_sync_index  = 0;
		int32_t m_image_index = -1;
		bool m_need_recreate  = false;

		VulkanSwapchain(Window* window, bool vsync);
		~VulkanSwapchain();
		VulkanSwapchain& vsync(bool flag, bool is_init = false);

		VulkanSwapchain& create(vk::SwapchainKHR* old = nullptr);
		VulkanSwapchain& release();
		VulkanSwapchain& recreate();

		int_t acquire_image_index(VulkanCommandBuffer* cmd_buffer);
		int_t do_present(VulkanCommandBuffer* cmd_buffer);
		VulkanSwapchainRenderTarget* backbuffer();
		int_t try_present(int_t (VulkanSwapchain::*callback)(VulkanCommandBuffer*), VulkanCommandBuffer* cmd_buffer,
		                  bool skip_on_out_of_date);

		vk::Semaphore render_finished_semaphore();
		vk::Semaphore image_present_semaphore();
	};

	struct VulkanViewport : public VulkanDeferredDestroy<RHI_Viewport> {
		std::vector<VkImageView> m_image_views;
		VulkanSwapchain* m_swapchain = nullptr;

		vk::Image current_image();
		vk::ImageLayout default_image_layout();
		VulkanSwapchainRenderTarget* render_target();

		VulkanViewport* init(WindowRenderViewport* viewport, bool vsync);

		void destroy_image_views();
		void present() override;

		void on_resize(const Size2D& new_size) override;
		void vsync(bool flag) override;
		void bind() override;
		void blit_target(RHI_RenderTargetView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		                 RHISamplerFilter filter) override;
		void clear_color(const LinearColor& color) override;

		~VulkanViewport();
	};
}// namespace Engine
