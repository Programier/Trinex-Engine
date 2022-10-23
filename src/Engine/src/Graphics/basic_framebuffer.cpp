#include <Graphics/basic_framebuffer.hpp>
#include <api_funcs.hpp>
#include <opengl.hpp>

namespace Engine
{
    const BasicFrameBuffer& BasicFrameBuffer::clear_buffer(const BufferType& type) const
    {
        clear_frame_buffer(_M_ID, type);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::bind() const
    {
        bind_framebuffer(_M_ID);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::view_port(const Point2D& pos, const Point2D& size) const
    {
        bind();
        set_framebuffer_viewport(_M_ID, pos, size);
        return *this;
    }

    FrameBufferType BasicFrameBuffer::type() const
    {
        return _M_type;
    }

}// namespace Engine
