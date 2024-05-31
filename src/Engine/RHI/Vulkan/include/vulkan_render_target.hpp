#pragma once
#include <bitset>

#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    struct VulkanRenderTargetState {
        struct VulkanRenderPass* m_render_pass = nullptr;
        vk::RenderPassBeginInfo m_render_pass_info;
        vk::Extent2D m_size;
        vk::Rect2D m_scissor;
        vk::Viewport m_viewport;

        void init(const RenderTarget* render_target, VulkanRenderPass* render_pass);
        void post_init();

        virtual bool is_main_render_target_state();
    };

    struct VulkanMainRenderTargetState : public VulkanRenderTargetState {
        bool is_main_render_target_state() override;
    };


    struct VulkanRenderTargetBase : RHI_RenderTarget {
        vk::Framebuffer m_framebuffer;
        VulkanRenderTargetState* m_state = nullptr;

        virtual bool is_main_render_target();
        virtual VulkanRenderTargetBase& destroy();

        VulkanRenderTargetBase& post_init(const Vector<vk::ImageView>& image_views);
        VulkanRenderTargetBase& size(uint32_t width, uint32_t height);

        void bind() override;
        VulkanRenderTargetBase& unbind();

        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        VulkanRenderTargetBase& update_viewport();
        VulkanRenderTargetBase& update_scissors();

        void clear_color(const ColorClearValue& color, byte layout) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        ~VulkanRenderTargetBase();
    };


    struct VulkanWindowRenderTargetFrame : VulkanRenderTargetBase {
        bool is_main_render_target() override;
    };

    struct VulkanRenderTarget : VulkanRenderTargetBase {
        VulkanRenderTargetState state;
        Vector<vk::ImageView> m_attachments;
        Vector<struct VulkanTexture*> m_color_textures;
        struct VulkanTexture* m_depth_texture = nullptr;

        VulkanRenderTarget();
        VulkanRenderTarget& init(const RenderTarget* info, VulkanRenderPass* render_pass);
        VulkanRenderTarget& on_color_attachment(struct VulkanTexture* texture, Index index);
        VulkanRenderTarget& on_depth_stencil_attachment(struct VulkanTexture* texture, Index index);
        VulkanRenderTarget& destroy() override;

        void clear_color(const ColorClearValue& color, byte layout) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;

        ~VulkanRenderTarget();
    };


    struct VulkanWindowRenderTarget : RHI_RenderTarget {
        VulkanMainRenderTargetState state;
        Vector<VulkanWindowRenderTargetFrame*> m_frames;
        struct VulkanWindowViewport* m_viewport;

        VulkanWindowRenderTarget& init(struct VulkanWindowViewport* viewport);
        VulkanWindowRenderTarget& destroy();

        void resize_count(size_t new_count);
        bool is_destroyable() const override;
        VulkanWindowRenderTargetFrame* frame();

        void bind() override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        void clear_color(const ColorClearValue& color, byte layout) override;

        ~VulkanWindowRenderTarget();
    };
}// namespace Engine
