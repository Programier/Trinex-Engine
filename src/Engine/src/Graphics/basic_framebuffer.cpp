#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <api.hpp>

namespace Engine
{

    implement_class(BasicFrameBuffer, "Engine");
    implement_default_initialize_class(BasicFrameBuffer);


    BasicFrameBuffer::BasicFrameBuffer()
    {}


    const BasicFrameBuffer& BasicFrameBuffer::bind(size_t buffer_index) const
    {
        if (_M_rhi_framebuffer)
            _M_rhi_framebuffer->bind(buffer_index);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::viewport(const ViewPort& viewport)
    {
        if (&_M_viewport != &viewport)
            _M_viewport = viewport;
        if (_M_rhi_framebuffer)
            _M_rhi_framebuffer->viewport(viewport);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::clear_color(const ColorClearValue& color, byte layout) const
    {
        if (_M_rhi_framebuffer)
            _M_rhi_framebuffer->clear_color(color, layout);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::scissor(const Scissor& scissor)
    {
        if (_M_rhi_framebuffer)
            _M_rhi_framebuffer->scissor(scissor);

        if (&scissor != &_M_scissor)
            _M_scissor = scissor;
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::clear_depth_stencil(const DepthStencilClearValue& value) const
    {
        if (_M_rhi_framebuffer)
            _M_rhi_framebuffer->clear_depth_stencil(value);
        return *this;
    }

    const ViewPort& BasicFrameBuffer::viewport()
    {
        return _M_viewport;
    }

    const Scissor& BasicFrameBuffer::scissor()
    {
        return _M_scissor;
    }
}// namespace Engine
