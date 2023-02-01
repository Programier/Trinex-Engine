#include <opengl_api.hpp>
#include <opengl_types.hpp>


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


namespace Engine
{
    struct OpenGL_FrameBuffer : public OpenGL_Object {
        GLuint _M_type;

        OpenGL_FrameBuffer(FrameBufferType type)
        {
            OpenGL::_M_api->_M_current_logger->log("OpenGL: Creating new FrameBuffer\n");
            _M_type = _M_framebuffer_types.at(type);

            glGenFramebuffers(1, &_M_instance_id);
        }

        ~OpenGL_FrameBuffer()
        {
            OpenGL::_M_api->_M_current_logger->log("OpenGL: Destroy FrameBuffer\n");
            glDeleteFramebuffers(1, &_M_instance_id);
        }
    };

    OpenGL& OpenGL::gen_framebuffer(ObjID& ID, FrameBufferType type)
    {
        if (ID)
            destroy_object(ID);
        ID = get_object_id(new OpenGL_FrameBuffer(type));
        return *this;
    }

    OpenGL& OpenGL::clear_frame_buffer(const ObjID& ID, BufferType type)
    {
        OpenGL::bind_framebuffer(ID);
        glClear(get_buffer_byte(type));

        return *this;
    }
    OpenGL& OpenGL::bind_framebuffer(const ObjID& ID)
    {
        if (ID)
        {
            check(ID, *this);
            auto buffer = obj->get_instance_by_type<OpenGL_FrameBuffer>();
            glBindFramebuffer(buffer->_M_type, buffer->_M_instance_id);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        return *this;
    }

    OpenGL& OpenGL::framebuffer_viewport(const Point2D& pos, const Size2D& size)
    {
        glViewport(static_cast<GLint>(pos.x), static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x),
                   static_cast<GLsizei>(size.y));
        return *this;
    }

    OpenGL& OpenGL::attach_texture_to_framebuffer(const ObjID& ID, const ObjID& _texture, FrameBufferAttach attach,
                                                  TextureAttachIndex num, int level)
    {
        if (!_texture)
            return *this;

        check(ID, *this);
        auto buffer = obj->get_instance_by_type<OpenGL_FrameBuffer>();

        glBindFramebuffer(buffer->_M_type, buffer->_M_instance_id);

        if (_texture)
        {
            glFramebufferTexture2D(buffer->_M_type,
                                   _M_framebuffer_attach.at(attach) +
                                           (attach == FrameBufferAttach::COLOR_ATTACHMENT ? num : 0),
                                   get_gl_type_of_texture(_texture), texture_id(_texture), level);
        }
        else
        {
            glFramebufferTexture2D(buffer->_M_type,
                                   _M_framebuffer_attach.at(attach) +
                                           (attach == FrameBufferAttach::COLOR_ATTACHMENT ? num : 0),
                                   GL_TEXTURE_2D, 0, level);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
        {
            _M_current_logger->log("Incomplete framebuffer attachments\n");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
        {
            _M_current_logger->log("incomplete missing framebuffer attachments");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
        {
            _M_current_logger->log("Incomplete framebuffer attachments dimensions\n");
        }
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
        {
            _M_current_logger->log(
                    "Combination of internal formats used by attachments in thef ramebuffer results in a "
                    "nonrednerable target");
        }
        else
        {
            _M_current_logger->log("Framebuffer: Attach success!\n");
        }
        return *this;
    }
}// namespace Engine
