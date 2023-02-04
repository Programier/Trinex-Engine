#include <Core/engine.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <api.hpp>

namespace Engine
{
    declare_instance_info_cpp(BasicFrameBuffer);
    constructor_cpp(BasicFrameBuffer)
    {}

    const BasicFrameBuffer& BasicFrameBuffer::clear_buffer(const BufferType& type) const
    {
        EngineInstance::instance()->api_interface()->clear_frame_buffer(_M_ID, type);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::bind() const
    {
        EngineInstance::instance()->api_interface()->bind_framebuffer(_M_ID);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::view_port(const Point2D& pos, const Point2D& size) const
    {
        EngineInstance::instance()->api_interface()->framebuffer_viewport(pos, size);
        return *this;
    }

    FrameBufferType BasicFrameBuffer::type() const
    {
        return _M_type;
    }

}// namespace Engine
