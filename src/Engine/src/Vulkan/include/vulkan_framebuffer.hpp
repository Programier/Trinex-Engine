#pragma once
#include <vulkan_object.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct VulkanFramebuffer : VulkanObject {
        vk::Framebuffer _M_framebuffer;
        vk::RenderPass _M_render_pass;
        vk::RenderPassBeginInfo _M_render_pass_info;
        vk::ClearValue _M_clear_color;

        bool _M_destroy_render_pass = false;

        struct ImageViewData {
            vk::ImageView _M_image_view;
            vk::ImageLayout _M_layout;
        };

        std::vector<ImageViewData> _M_image_views;

        VulkanFramebuffer& create_render_pass();

        void* get_instance_data() override;
        VulkanFramebuffer& generate();
        VulkanFramebuffer& destroy();
        VulkanFramebuffer& bind();
        VulkanFramebuffer& unbind();
        VulkanFramebuffer& init_render_pass_info();
        VulkanFramebuffer& update_viewport();
        VulkanFramebuffer& clear_color(BufferType type);
        ~VulkanFramebuffer();
    };
}// namespace Engine
