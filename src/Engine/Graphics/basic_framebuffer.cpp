#include <GL/glew.h>
#include <Graphics/basic_framebuffer.hpp>
#include <opengl.hpp>

namespace Engine
{
    const BasicFrameBuffer& BasicFrameBuffer::clear_buffer(const BufferType& type) const
    {
        int buffer = Engine::OpenGL::get_buffer(type);
        if (buffer != 0)
            glClear(buffer);
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _M_framebuffer_id.get());
        return *this;
    }

    const BasicFrameBuffer& BasicFrameBuffer::view_port(const Point2D& pos, const Point2D& size) const
    {
        bind();
        glViewport(static_cast<GLint>(pos.x), static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x),
                   static_cast<GLsizei>(size.y));
        return *this;
    }

}// namespace Engine
