#pragma once
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
    struct VulkanViewport : public RHI_Viewport {

        struct SyncObject {
            vk::Semaphore _M_image_present;
            vk::Semaphore _M_render_finished;
            vk::Fence _M_fence;

            SyncObject();
            ~SyncObject();
        };

        Vector<vk::CommandBuffer> _M_command_buffers;
        Vector<SyncObject> _M_sync_objects;

        std::vector<VkImage> _M_images;
        std::vector<VkImageView> _M_image_views;
        vkb::Swapchain* _M_swapchain = nullptr;
        vk::SurfaceKHR _M_surface;
        struct VulkanRenderTarget* _M_render_target = nullptr;

        vk::PresentModeKHR _M_present_mode;
        uint32_t _M_buffer_index = 0;

        bool _M_need_recreate_swap_chain = false;

        void init();
        void create_main_render_target();
        void create_swapchain();
        void destroy(bool fully = false);
        void destroy_swapchain(bool fully = false);
        void recreate_swapchain();
        vk::ResultValue<uint32_t> swapchain_image_index();

        VulkanViewport* init(WindowInterface* window, bool vsync, bool create_render_pass);
        VulkanViewport* init(RenderTarget* render_target);


        void begin_render_window();
        void begin_render_render_target();
        void begin_render() override;
        void end_render() override;
        void on_resize(const Size2D& new_size) override;
        bool vsync() override;
        void vsync(bool flag) override;
        RHI_RenderTarget* render_target() override;

        ~VulkanViewport() override;
    };


    struct VulkanRenderTargetViewport : VulkanViewport {
    };

    struct VulkanWindowViewport : VulkanViewport {
    };
}// namespace Engine
