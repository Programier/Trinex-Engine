#pragma once
#include <bitset>

#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{

    struct VulkanFramebuffer : RHI_RenderTarget {
        vk::Framebuffer _M_framebuffer;
        Vector<vk::ImageView> _M_attachments;
        vk::RenderPassBeginInfo _M_render_pass_info;
        vk::CommandBuffer _M_command_buffer;
        vk::Semaphore _M_render_finished;
        vk::Fence _M_fence;

        vk::Extent2D _M_size;
        vk::Rect2D _M_scissor;
        vk::Viewport _M_viewport;

        Vector<vk::ClearValue> _M_clear_values = {
                vk::ClearValue(vk::ClearColorValue(Array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})))};

        struct VulkanRenderPass* _M_render_pass = nullptr;
        bool _M_is_inited                       = false;

        VulkanFramebuffer();
        VulkanFramebuffer& init(const RenderTarget* info, VulkanRenderPass* render_pass);

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

        VulkanFramebuffer& begin_pass();
        VulkanFramebuffer& end_pass();

        bool is_custom() const;

        void bind() override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;

        virtual bool is_main_framebuffer() const;

        ~VulkanFramebuffer();
    };

    struct VulkanMainFrameBufferFrame : public VulkanFramebuffer
    {
        bool is_main_framebuffer() const override;
    };


    struct VulkanMainFrameBuffer : RHI_RenderTarget {
        Vector<VulkanMainFrameBufferFrame*> _M_framebuffers;

        VulkanMainFrameBuffer();
        VulkanMainFrameBuffer& init();

        VulkanMainFrameBuffer& destroy();
        void resize_count(size_t new_count);
        VulkanMainFrameBuffer& size(uint32_t width, uint32_t height);

        void bind() override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        void clear_color(const ColorClearValue& color, byte layout) override;

        bool is_destroyable() const override;

        ~VulkanMainFrameBuffer();
    };
}// namespace Engine
