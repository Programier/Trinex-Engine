#pragma once
#include <bitset>

#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{

    struct VulkanRenderTargetFrame {
        vk::Framebuffer _M_framebuffer;
        Vector<vk::ImageView> _M_attachments;
        struct VulkanRenderTarget* _M_owner = nullptr;


        void init(struct VulkanRenderTarget* owner, const RenderTarget* info, struct VulkanRenderPass* render_pass,
                  Index frame);
        void post_init();
        void destroy();

        static void push_barriers(size_t count);
        void bind();
        void unbind();
        VulkanRenderTargetFrame& update_viewport();
        VulkanRenderTargetFrame& update_scissors();


        virtual bool is_main_frame();
        virtual ~VulkanRenderTargetFrame();
    };

    struct VulkanWindowRenderTargetFrame : public VulkanRenderTargetFrame {
        VulkanWindowRenderTargetFrame();
        ~VulkanWindowRenderTargetFrame();

        bool is_main_frame() override;
    };


    struct VulkanRenderTarget : RHI_RenderTarget {
        Vector<VulkanRenderTargetFrame*> _M_frames;
        struct VulkanRenderPass* _M_render_pass = nullptr;

        vk::RenderPassBeginInfo _M_render_pass_info;
        vk::Extent2D _M_size;
        vk::Rect2D _M_scissor;
        vk::Viewport _M_viewport;

        Vector<vk::ClearValue> _M_clear_values = {
                vk::ClearValue(vk::ClearColorValue(Array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})))};


        VulkanRenderTarget& init(const RenderTarget* info, VulkanRenderPass* render_pass);
        VulkanRenderTarget& post_init();
        VulkanRenderTarget& destroy();
        VulkanRenderTarget& size(uint32_t width, uint32_t height);

        Index bind() override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        void clear_color(const ColorClearValue& color, byte layout) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;

        virtual size_t buffer_index() const;


        virtual bool is_main_render_target();

        ~VulkanRenderTarget();
    };


    struct VulkanWindowRenderTarget : VulkanRenderTarget {

        struct VulkanWindowViewport* _M_viewport;


        VulkanWindowRenderTarget& init(struct VulkanWindowViewport* viewport);
        VulkanWindowRenderTarget& destroy();

        size_t buffer_index() const override;

        void resize_count(size_t new_count);
        bool is_destroyable() const override;

        bool is_main_render_target() override;
    };
}// namespace Engine