#pragma once
#include <Core/api_object.hpp>
#include <Core/structures.hpp>


namespace Engine
{
    class RenderPass;

    class ENGINE_EXPORT BasicRenderTarget : public ApiObject
    {
        declare_class(BasicRenderTarget, ApiObject);

    protected:
        ViewPort _M_viewport;
        Scissor _M_scissor;

    public:
        RenderPass* render_pass = nullptr;

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
