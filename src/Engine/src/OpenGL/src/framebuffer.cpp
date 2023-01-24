#include <opengl_framebuffer.hpp>
#include <opengl_texture.hpp>
#include <opengl_types.hpp>


void OpenGL_FrameBuffer::destroy()
{
    glDeleteFramebuffers(1, &_M_ID);
    DEALLOC_INFO;
}

declare_cpp_destructor(OpenGL_FrameBuffer);


API void api_gen_framebuffer(ObjID& ID, FrameBufferType type)
{
    if (ID)
        api_destroy_object_instance(ID);

    OpenGL_FrameBuffer* buffer = new OpenGL_FrameBuffer();
    buffer->_M_type = _M_framebuffer_types.at(type);

    glGenFramebuffers(1, &buffer->_M_ID);
    ID = object_id_of(buffer);
}

API void api_bind_framebuffer(const ObjID& ID)
{
    if (ID)
    {
        make_frame_buffer(buffer, ID);
        check(buffer, );

        glBindFramebuffer(buffer->_M_type, buffer->_M_ID);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

API void api_set_framebuffer_viewport(const ObjID& ID, const Point2D& pos, const Size2D& size)
{
    api_bind_framebuffer(ID);
    glViewport(static_cast<GLint>(pos.x), static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x),
               static_cast<GLsizei>(size.y));
}


API void api_attach_texture_to_framebuffer(const ObjID& ID, const ObjID& _texture, FrameBufferAttach attach,
                                           unsigned int num, int level)
{
    make_frame_buffer(buffer, ID);
    check(buffer, );

    glBindFramebuffer(buffer->_M_type, buffer->_M_ID);

    if (_texture)
    {
        glFramebufferTexture2D(buffer->_M_type,
                               _M_framebuffer_attach.at(attach) + (attach == FrameBufferAttach::COLOR_ATTACHMENT ? num : 0),
                               texture(_texture)->_M_GL_type, texture(_texture)->_M_ID, level);
    }
    else
    {
        glFramebufferTexture2D(buffer->_M_type,
                               _M_framebuffer_attach.at(attach) + (attach == FrameBufferAttach::COLOR_ATTACHMENT ? num : 0),
                               GL_TEXTURE_2D, 0, level);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
    {
        external_logger->log("Incomplete framebuffer attachments\n");
    }
    else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
    {
        external_logger->log("incomplete missing framebuffer attachments");
    }
    else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
    {
        external_logger->log("Incomplete framebuffer attachments dimensions\n");
    }
    else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
    {
        external_logger->log(
                "Combination of internal formats used by attachments in thef ramebuffer results in a nonrednerable target");
    }
    else
    {
        external_logger->log("Framebuffer: Attach success!\n");
    }
}


static int get_buffer_byte(const BufferType& buffer)
{
    int opengl_buffer = 0;
    if ((buffer & COLOR_BUFFER_BIT) == COLOR_BUFFER_BIT)
        opengl_buffer |= GL_COLOR_BUFFER_BIT;

    if ((buffer & DEPTH_BUFFER_BIT) == DEPTH_BUFFER_BIT)
        opengl_buffer |= GL_DEPTH_BUFFER_BIT;

    if ((buffer & STENCIL_BUFFER_BIT) == STENCIL_BUFFER_BIT)
        opengl_buffer |= GL_STENCIL_BUFFER_BIT;
    return opengl_buffer;
}


API void api_clear_frame_buffer(const ObjID& ID, BufferType type)
{
    api_bind_framebuffer(ID);
    glClear(get_buffer_byte(type));
}
