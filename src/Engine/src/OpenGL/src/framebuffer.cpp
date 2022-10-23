#include <opengl_framebuffer.hpp>
#include <opengl_texture.hpp>
#include <opengl_types.hpp>


//extern void (*clear_frame_buffer)(const ObjID& _M_ID, BufferType type);


API void destroy_framebuffer(ObjID& ID)
{
    check_id(ID, );
    make_frame_buffer(buffer, ID);

    if (buffer->_M_ID)
    {
        glDeleteFramebuffers(1, &buffer->_M_ID);
        buffer->_M_ID = 0;
        delete buffer;
    }
}


API void api_gen_framebuffer(ObjID& ID, FrameBufferType type)
{
    if (ID)
        api_destroy_object_instance(ID);

    OpenGL_Object* object = new OpenGL_Object();
    object->_M_references = 1;
    object->_M_type = Type::FRAMEBUFFER;

    OpenGL_FrameBuffer* buffer = new OpenGL_FrameBuffer();
    object->_M_data = static_cast<void*>(buffer);
    buffer->_M_type = _M_framebuffer_types.at(type);

    glGenFramebuffers(1, &buffer->_M_ID);
    ID = reinterpret_cast<ObjID>(object);
}

API void api_bind_framebuffer(const ObjID& ID)
{
    if (ID)
    {
        make_frame_buffer(buffer, ID);
        glBindFramebuffer(buffer->_M_type, buffer->_M_ID);
    }
    else
    {
        glBindFramebuffer(GL_RENDERBUFFER, 0);
    }
}

API void api_set_framebuffer_viewport(const ObjID& ID, const Point2D& pos, const Size2D& size)
{
    api_bind_framebuffer(ID);
    glViewport(static_cast<GLint>(pos.x), static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x),
               static_cast<GLsizei>(size.y));
}


//#include <Core/logger.hpp>
API void api_attach_texture_to_framebuffer(const ObjID& ID, const ObjID& _texture, FrameBufferAttach attach,
                                           unsigned int num, int level)
{
    check_id(ID, );

    make_frame_buffer(buffer, ID);
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
    glReadBuffer(GL_NONE);
}


static int get_buffer_byte(const BufferType& buffer)
{
    int opengl_buffer = 0;
    if ((buffer & COLOR_BUFFER_BIT) == COLOR_BUFFER_BIT)
        opengl_buffer |= GL_COLOR_BUFFER_BIT;

    if ((buffer & DEPTH_BUFFER_BIT) == DEPTH_BUFFER_BIT)
        opengl_buffer |= GL_DEPTH_BUFFER_BIT;
    return opengl_buffer;
}


API void api_clear_frame_buffer(const ObjID& ID, BufferType type)
{
    api_bind_framebuffer(ID);
    glClear(get_buffer_byte(type));
}
