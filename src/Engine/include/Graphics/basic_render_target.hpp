#pragma once
#include <Core/render_resource.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>


namespace Engine
{
    class RenderPass;

    class ENGINE_EXPORT BasicRenderTarget : public RenderResource
    {
        declare_class(BasicRenderTarget, RenderResource);

    protected:
        ViewPort _M_viewport;
        Scissor _M_scissor;

    public:
        Pointer<RenderPass> render_pass;

        delete_copy_constructors(BasicRenderTarget);
        BasicRenderTarget();
        const BasicRenderTarget& bind(size_t buffer_index = 0) const;
        const BasicRenderTarget& viewport(const ViewPort& viewport);
        const BasicRenderTarget& scissor(const Scissor& scissor);
        const ViewPort& viewport();
        const Scissor& scissor();
        const BasicRenderTarget& clear_color(const ColorClearValue& color, byte layout = 0) const;
        const BasicRenderTarget& clear_depth_stencil(const DepthStencilClearValue& value) const;
    };
}// namespace Engine