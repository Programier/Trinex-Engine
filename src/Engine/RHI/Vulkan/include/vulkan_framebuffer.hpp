#pragma once
#include <bitset>

#include <api.hpp>
#include <vulkan_object.hpp>

namespace Engine
{

    struct VulkanFramebuffer : RHI::RHI_FrameBuffer {
        struct Buffer {
            vk::Framebuffer _M_framebuffer;
            Vector<vk::ImageView> _M_attachments;
        };

        Vector<vk::AttachmentDescription> _M_attachment_descriptions;
        Vector<vk::AttachmentReference> _M_color_attachment_references;
        vk::AttachmentReference* _M_depth_attachment_renference = nullptr;

        Vector<Buffer> _M_buffers;

        vk::RenderPass _M_render_pass;
        vk::RenderPassBeginInfo _M_render_pass_info;

        bool _M_is_custom = true;
        bool _M_is_inited = false;
        vk::SubpassDescription _M_subpass;
        vk::SubpassDependency _M_dependency;
        vk::Extent2D _M_size;
        vk::Rect2D _M_scissor;
        vk::Viewport _M_viewport;
        Vector<vk::ClearValue> _M_clear_values = {
                vk::ClearValue(vk::ClearColorValue(Array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})))};


        VulkanFramebuffer();
        VulkanFramebuffer& init(const FrameBufferCreateInfo& info);

        VulkanFramebuffer& create_render_pass();
        VulkanFramebuffer& create_framebuffer();
        VulkanFramebuffer& destroy();
        VulkanFramebuffer& unbind();
        VulkanFramebuffer& set_viewport();
        VulkanFramebuffer& set_scissor();
        VulkanFramebuffer& init_render_pass_info();
        void clear_color(const ColorClearValue& color, byte layout) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        VulkanFramebuffer& size(uint32_t width, uint32_t height);

        VulkanFramebuffer& begin_pass(size_t index);
        VulkanFramebuffer& end_pass();

        void bind(uint_t buffer_index) override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;

        ~VulkanFramebuffer();
    };


    struct VulkanMainFrameBuffer : VulkanFramebuffer {
        void bind(uint_t) override;
    };
}// namespace Engine
