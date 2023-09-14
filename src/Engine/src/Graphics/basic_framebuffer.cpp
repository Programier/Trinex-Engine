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
        EngineInstance::instance()->api_interface()->bind_framebuffer(_M_ID, buffer_index);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::viewport(const ViewPort& viewport)
    {
        if (&_M_viewport != &viewport)
            _M_viewport = viewport;
        EngineInstance::instance()->api_interface()->framebuffer_viewport(_M_ID, viewport);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::clear_color(const ColorClearValue& color, byte layout) const
    {
        EngineInstance::instance()->api_interface()->clear_color(_M_ID, color, layout);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::scissor(const Scissor& scissor)
    {
        EngineInstance::instance()->api_interface()->framebuffer_scissor(_M_ID, scissor);

        if (&scissor != &_M_scissor)
            _M_scissor = scissor;
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::clear_depth_stencil(const DepthStencilClearValue& value) const
    {
        EngineInstance::instance()->api_interface()->clear_depth_stencil(_M_ID, value);
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
