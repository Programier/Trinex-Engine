#include <GL/glew.h>
#include <Graphics/framebuffer.hpp>

namespace Engine
{
    FrameBuffer::FrameBuffer() = default;

    FrameBuffer::FrameBuffer(const Size1D& width, const Size1D& height)
    {
        gen(width, height);
    }

    FrameBuffer::FrameBuffer(const Size2D& size)
    {
        gen(size.x, size.y);
    }


    FrameBuffer::FrameBuffer(const FrameBuffer& buffer) = default;
    FrameBuffer& FrameBuffer::delete_data()
    {

        if (_M_framebuffer_id.references() == 1 && _M_framebuffer_id.get() != 0)
            glDeleteFramebuffers(1, &_M_framebuffer_id.get());
        if (_M_texture_id.references() == 1 && _M_texture_id.get() != 0)
            glDeleteTextures(1, &_M_texture_id.get());
        return *this;
    }

    FrameBuffer& FrameBuffer::operator=(const FrameBuffer& buffer) = default;


    FrameBuffer& FrameBuffer::gen(const Size1D& width, const Size1D& height)
    {
        ObjectID FB_ID;
        glGenFramebuffers(1, &FB_ID);

        ObjectID TEX_ID;
        glGenTextures(1, &TEX_ID);
        glBindTexture(GL_TEXTURE_2D, TEX_ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindFramebuffer(GL_FRAMEBUFFER, FB_ID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TEX_ID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        delete_data();
        _M_framebuffer_id = ReferenceWrapper(const_cast<const unsigned int&>(FB_ID));
        _M_texture_id = ReferenceWrapper(const_cast<const unsigned int&>(TEX_ID));
        _M_size = {width, height};
        return *this;
    }

    FrameBuffer& FrameBuffer::gen(const Size2D& size)
    {
        return gen(size.x, size.y);
    }


    FrameBuffer& FrameBuffer::bind_texture(int num)
    {
        glActiveTexture(GL_TEXTURE0 + num);
        glBindTexture(GL_TEXTURE_2D, _M_texture_id.get());
        return *this;
    }

    Size2D FrameBuffer::size() const
    {
        return _M_size;
    }

    FrameBuffer::~FrameBuffer()
    {
        delete_data();
    }
}// namespace Engine
