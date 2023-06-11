#pragma once
#include <Core/api_object.hpp>
#include <Core/buffer_types.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/render_types.hpp>


namespace Engine
{
    class ENGINE_EXPORT BasicFrameBuffer : public ApiObject
    {
    protected:
        ViewPort _M_viewport;
        Scissor _M_scissor;

    public:
        using Super = ApiObject;

        delete_copy_constructors(BasicFrameBuffer);
        BasicFrameBuffer();
        const BasicFrameBuffer& bind(size_t buffer_index = 0) const;
        const BasicFrameBuffer& view_port(const ViewPort& viewport);
        const BasicFrameBuffer& scissor(const Scissor& scissor);
        const ViewPort& viewport();
        const Scissor& scissor();
        const BasicFrameBuffer& clear_color(const ColorClearValue& color, byte layout = 0) const;
        const BasicFrameBuffer& clear_depth_stencil(const DepthStencilClearValue& value) const;
    };
}// namespace Engine
