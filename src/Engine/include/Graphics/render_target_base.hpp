#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>
#include <Core/structures.hpp>


namespace Engine
{
    class RenderPass;

    class ENGINE_EXPORT RenderTargetBase : public RenderResource
    {
        declare_class(RenderTargetBase, RenderResource);

    protected:
        static RenderTargetBase* m_current_target;
        static RenderPass* m_current_pass;

        ViewPort m_viewport;
        Scissor m_scissor;
        byte m_frame_index = -1;

    public:
        RenderPass* render_pass;

        delete_copy_constructors(RenderTargetBase);
        RenderTargetBase();

        RenderTargetBase& rhi_bind(RenderPass* render_pass = nullptr);
        const RenderTargetBase& viewport(const ViewPort& viewport);
        const RenderTargetBase& scissor(const Scissor& scissor);
        const ViewPort& viewport() const;
        const Scissor& scissor() const;
        const RenderTargetBase& clear_color(const ColorClearValue& color, byte layout = 0) const;
        const RenderTargetBase& clear_depth_stencil(const DepthStencilClearValue& value) const;
        RenderTargetBase& rhi_create() override;
        virtual Size2D render_target_size() const = 0;
        byte frame_index() const;
        static RenderTargetBase* current_target();
        static RenderPass* current_render_pass();
        static void reset_current_target();

        ~RenderTargetBase();
    };
}// namespace Engine
