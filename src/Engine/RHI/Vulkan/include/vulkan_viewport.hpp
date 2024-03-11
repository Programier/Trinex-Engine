#pragma once
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
    struct VulkanViewport : public RHI_Viewport {

        struct SyncObject {
            vk::Semaphore m_image_present;
            vk::Semaphore m_render_finished;
            vk::Fence m_fence;

            SyncObject();
            ~SyncObject();
        };

        Vector<vk::CommandBuffer> m_command_buffers;
        Vector<SyncObject> m_sync_objects;
        std::vector<VkImageView> m_image_views;
        struct RHI_RenderTarget* m_render_target = nullptr;
        uint32_t m_buffer_index                  = 0;


        void init();
        void reinit();
        void destroy_image_views();
        void before_begin_render();
        void after_end_render();

        void begin_render() override;
        void end_render() override;

        void on_resize(const Size2D& new_size) override;
        Identifier internal_type() override;
        bool vsync() override;
        void vsync(bool flag) override;
        RHI_RenderTarget* render_target() override;
        ~VulkanViewport() override;
    };


    struct VulkanRenderTargetViewport : VulkanViewport {
        VulkanViewport* init(RenderTarget* render_target);

        void begin_render() override;
        void end_render() override;
    };

    struct VulkanWindowViewport : VulkanViewport {

        vk::PresentModeKHR m_present_mode;
        WindowInterface* m_window   = nullptr;
        vkb::Swapchain* m_swapchain = nullptr;
        vk::SurfaceKHR m_surface;
        std::vector<VkImage> m_images;
        bool m_need_recreate_swap_chain = false;

        VulkanViewport* init(WindowInterface* window, bool vsync, bool need_initialize);

        void create_main_render_target();
        void create_swapchain();
        void destroy_swapchain(bool fully = false);
        void recreate_swapchain();
        vk::ResultValue<uint32_t> swapchain_image_index();


        void begin_render() override;
        void end_render() override;
        void on_resize(const Size2D& new_size) override;
        bool vsync() override;
        void vsync(bool flag) override;

        ~VulkanWindowViewport();
    };
}// namespace Engine
