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

	struct VulkanWindowViewport : VulkanViewport {
		struct SyncObject : VulkanViewport::SyncObject {
			vk::Semaphore m_image_present;
			vk::Semaphore m_render_finished;
			VulkanCommandBuffer* m_cmd_buffer = nullptr;

			SyncObject();
			~SyncObject();

			inline vk::Semaphore* image_present() override
			{
				return &m_image_present;
			}

			inline vk::Semaphore* render_finished() override
			{
				return &m_render_finished;
			}
		};

		Vector<SyncObject> m_sync_objects;
		struct VulkanWindowRenderTarget* m_render_target = nullptr;
		vkb::Swapchain* m_swapchain                      = nullptr;

		Vector<VkImage> m_images;
		vk::SurfaceKHR m_surface;
		uint32_t m_buffer_index = 0;

		bool m_need_recreate_swap_chain = false;
		bool m_vsync                    = false;

		SyncObject* current_sync_object() override;
		vk::Image current_image() override;
		vk::ImageLayout default_image_layout() override;
		VulkanRenderTargetBase* render_target() override;
		bool is_window_viewport() override;

		VulkanViewport* init(WindowRenderViewport* viewport, bool vsync);

		void create_render_target();
		void create_swapchain();
		void destroy_swapchain(bool fully = false);
		void recreate_swapchain();
		vk::ResultValue<uint32_t> swapchain_image_index();


		void begin_render() override;
		void end_render() override;
		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void vsync(bool flag) override;

		~VulkanWindowViewport();
	};
}// namespace Engine
