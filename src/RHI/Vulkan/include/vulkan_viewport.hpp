#pragma once
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	struct VulkanCommandBuffer;

	struct VulkanBackBuffer {
		vk::Semaphore m_image_present_semaphore;
		vk::Semaphore m_render_finished_semaphore;
		struct VulkanCommandBuffer* m_command_buffer        = nullptr;
		struct VulkanSwapchainRenderTarget* m_render_target = nullptr;

		VulkanBackBuffer& setup(vk::Image backbuffer, vk::ImageView view, Size2D size, vk::Format format);
		VulkanBackBuffer& wait_for_command_buffer();
		VulkanBackBuffer& release();
	};

	struct VulkanSwapchain {
		enum Status : int_t
		{
			Success     = 0,
			OutOfDate   = -1,
			SurfaceLost = -2,
		};

		Vector<VulkanBackBuffer> m_backbuffers;
		vk::SurfaceKHR m_surface;
		vk::PresentModeKHR m_present_mode;
		vk::SwapchainKHR m_swapchain;
		int32_t m_buffer_index = 0;
		int32_t m_image_index  = -1;
		bool m_need_recreate   = false;

		VulkanSwapchain(Window* window, bool vsync);
		~VulkanSwapchain();
		VulkanSwapchain& vsync(bool flag, bool is_init = false);

		VulkanSwapchain& create(vk::SwapchainKHR* old = nullptr);
		VulkanSwapchain& release();
		VulkanSwapchain& recreate();

		int_t acquire_image_index(VulkanCommandBuffer* cmd_buffer);
		int_t do_present(VulkanCommandBuffer* cmd_buffer);
		VulkanBackBuffer* backbuffer();
		int_t try_present(int_t (VulkanSwapchain::*callback)(VulkanCommandBuffer*), VulkanCommandBuffer* cmd_buffer,
						  bool skip_on_out_of_date);

		vk::Semaphore* render_finished_semaphore();
		vk::Semaphore* image_present_semaphore();
	};

	struct VulkanViewport : public RHI_DefaultDestroyable<RHI_Viewport> {
		std::vector<VkImageView> m_image_views;
		VulkanSwapchain* m_swapchain = nullptr;

		vk::Image current_image();
		vk::ImageLayout default_image_layout();
		VulkanRenderTargetBase* render_target();

		VulkanViewport* init(WindowRenderViewport* viewport, bool vsync);

		void destroy_image_views();
		void present() override;

		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void vsync(bool flag) override;
		void bind() override;
		void blit_target(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
						 SamplerFilter filter) override;
		void clear_color(const Color& color) override;

		~VulkanViewport();
	};
}// namespace Engine
