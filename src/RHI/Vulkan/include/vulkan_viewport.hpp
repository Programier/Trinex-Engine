#pragma once
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	struct VulkanCommandBuffer;

	struct VulkanViewport : public RHI_DefaultDestroyable<RHI_Viewport> {
		struct SyncObject {
			virtual vk::Semaphore* image_present();
			virtual vk::Semaphore* render_finished();
			virtual ~SyncObject();
		};

		std::vector<VkImageView> m_image_views;
		virtual SyncObject* current_sync_object()       = 0;
		virtual vk::Image current_image()               = 0;
		virtual vk::ImageLayout default_image_layout()  = 0;
		virtual VulkanRenderTargetBase* render_target() = 0;
		virtual bool is_window_viewport()               = 0;

		void destroy_image_views();

		void before_begin_render();
		void after_end_render();

		void begin_render() override;
		void end_render() override;

		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void vsync(bool flag) override;
		void bind() override;
		void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		void clear_color(const Color& color) override;
	};

	class Window;

	struct VulkanSurfaceViewport : VulkanViewport {
		RenderSurface* m_surface[1]                = {nullptr};
		struct VulkanRenderTarget* m_render_target = nullptr;
		SyncObject* m_sync_object;

		VulkanSurfaceViewport();
		~VulkanSurfaceViewport();

		SyncObject* current_sync_object() override;
		vk::Image current_image() override;
		vk::ImageLayout default_image_layout() override;
		VulkanRenderTargetBase* render_target() override;
		bool is_window_viewport() override;

		VulkanViewport* init(SurfaceRenderViewport* viewport);

		void begin_render() override;
		void end_render() override;
	};

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

		int_t acquire_image_index();
		int_t do_present();
		VulkanBackBuffer* backbuffer();
		int_t try_present(int_t (VulkanSwapchain::*callback)(), bool skip_on_out_of_date);

		vk::Semaphore* render_finished_semaphore();
		vk::Semaphore* image_present_semaphore();
	};

	struct VulkanWindowViewport : VulkanViewport {
		VulkanSwapchain* m_swapchain = nullptr;

		inline SyncObject* current_sync_object() override
		{
			static SyncObject obj;
			return &obj;
		}
		vk::Image current_image() override;
		vk::ImageLayout default_image_layout() override;
		VulkanRenderTargetBase* render_target() override;
		bool is_window_viewport() override;

		VulkanViewport* init(WindowRenderViewport* viewport, bool vsync);

		void begin_render() override;
		void end_render() override;
		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void vsync(bool flag) override;

		~VulkanWindowViewport();
	};
}// namespace Engine
