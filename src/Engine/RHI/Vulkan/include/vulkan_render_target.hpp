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
        Vector<vk::ClearValue> m_clear_values = {vk::ClearValue(vk::ClearColorValue(Array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})))};

        void init(const RenderTarget* render_target, VulkanRenderPass* render_pass);
        void post_init();

        virtual bool is_main_render_target_state();
    };

    struct VulkanMainRenderTargetState : public VulkanRenderTargetState {
        bool is_main_render_target_state() override;
    };


    struct VulkanRenderTargetBase : RHI_RenderTarget {
        vk::Framebuffer m_framebuffer;
        Vector<vk::ImageView> m_attachments;
        VulkanRenderTargetState* m_state = nullptr;

        virtual VulkanRenderTargetBase& init(const RenderTarget* info, VulkanRenderPass* render_pass);
        virtual bool is_main_render_target();
        virtual VulkanRenderTargetBase& destroy(bool called_by_destructor = false);

        VulkanRenderTargetBase& post_init();
        VulkanRenderTargetBase& size(uint32_t width, uint32_t height);

        void bind(RenderPass* render_pass) override;
        VulkanRenderTargetBase& unbind(VulkanRenderPass* next_render_pass = nullptr);
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

        VulkanRenderTarget();
        VulkanRenderTarget& init(const RenderTarget* info, VulkanRenderPass* render_pass) override;
        VulkanRenderTarget& destroy(bool called_by_destructor = false) override;
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

        void bind(RenderPass* render_pass) override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        void clear_color(const ColorClearValue& color, byte layout) override;

        ~VulkanWindowRenderTarget();
    };
}// namespace Engine
