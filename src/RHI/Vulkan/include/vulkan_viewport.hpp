#pragma once
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{

	struct VulkanViewport : public RHI_DefaultDestroyable<RHI_Viewport> {
		struct SyncObject {
			vk::Semaphore m_image_present;
			vk::Semaphore m_render_finished;
			vk::Fence m_fence;

			SyncObject();
			~SyncObject();
		};

		Vector<struct VulkanCommandBuffer> m_command_buffers;
		Vector<SyncObject> m_sync_objects;
		std::vector<VkImageView> m_image_views;
		uint32_t m_buffer_index = 0;


		void init();
		void reinit();
		void destroy_image_views();
		void before_begin_render();
		void after_end_render();

		void begin_render() override;
		void end_render() override;

		void on_resize(const Size2D& new_size) override;
		void vsync(bool flag) override;

		virtual VulkanRenderTargetBase* render_target();
		virtual bool is_window_viewport();

		~VulkanViewport() override;
	};

	class Window;
	struct VulkanWindowViewport : VulkanViewport {
		Vector<VkImage> m_images;
		RenderViewport* m_viewport                       = nullptr;
		struct VulkanWindowRenderTarget* m_render_target = nullptr;
		vkb::Swapchain* m_swapchain                      = nullptr;

		vk::SurfaceKHR m_surface;
		bool m_need_recreate_swap_chain = false;

		VulkanViewport* init(RenderViewport* viewport);

		void create_main_render_target();
		void create_swapchain();
		void destroy_swapchain(bool fully = false);
		void recreate_swapchain();
		vk::ResultValue<uint32_t> swapchain_image_index();


		void begin_render() override;
		void end_render() override;
		void on_resize(const Size2D& new_size) override;
		void vsync(bool flag) override;
		void bind() override;
		void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		void clear_color(const Color& color) override;
		VulkanRenderTargetBase* render_target() override;
		bool is_window_viewport() override;

		~VulkanWindowViewport();
	};
}// namespace Engine
