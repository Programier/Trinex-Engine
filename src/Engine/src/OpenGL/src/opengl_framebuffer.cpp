#include <opengl_api.hpp>
#include <opengl_framebuffer.hpp>
#include <opengl_texture.hpp>

namespace Engine
{
    implement_opengl_instance_cpp(OpenGL_FrameBuffer);
    implement_opengl_instance_cpp(OpenGL_FrameBufferSet);

    OpenGL_FrameBuffer::OpenGL_FrameBuffer() : _M_set(nullptr)
    {}

    static GLenum get_attachment_type(GLenum type)
    {
        switch (type)
        {
            case GL_DEPTH_STENCIL:
                return GL_DEPTH_STENCIL_ATTACHMENT;
            case GL_DEPTH_COMPONENT:
                return GL_DEPTH_ATTACHMENT;
            case GL_STENCIL_INDEX:
                return GL_STENCIL_ATTACHMENT;
            default:
                return GL_COLOR_ATTACHMENT0;
        }
    }

    OpenGL_FrameBufferSet::OpenGL_FrameBufferSet(bool is_base)
    {
        if (is_base)
        {
            _M_clear_color_values.push_back({0.0f, 0.0f, 0.0f, 1.f});

            _M_command_buffer.next(
                    new OpenGL_Command(glClearBufferfv, static_cast<GLenum>(GL_COLOR), static_cast<GLint>(0),
                                       reinterpret_cast<const GLfloat*>(_M_clear_color_values[0].data())));

            _M_command_buffer.next(new OpenGL_Command(glDisable, static_cast<GLenum>(GL_DEPTH_TEST)));
            _M_command_buffer.next(new OpenGL_Command(glDisable, static_cast<GLenum>(GL_STENCIL_TEST)));
            _M_framebuffers.resize(1);
            _M_framebuffers[0]._M_set         = this;
            _M_framebuffers[0]._M_instance_id = 0;
        }
    }

    OpenGL_FrameBuffer& OpenGL_FrameBuffer::attach_texture(const FrameBufferAttachment& texture_attachment,
                                                           GLenum attachment)
    {
        OpenGL_Texture* texture = GET_TYPE(OpenGL_Texture, texture_attachment.texture_id);

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->_M_texture_type, texture->_M_instance_id,
                               texture_attachment.mip_level);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
        {
            opengl_debug_log("Framebuffer", "Incomplete framebuffer attachments\n");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
        {
            opengl_debug_log("Framebuffer", "incomplete missing framebuffer attachments");
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
        {
            opengl_debug_log("Framebuffer", "Incomplete framebuffer attachments dimensions\n");
        }
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
        {
            opengl_debug_log("Framebuffer",
                             "Combination of internal formats used by attachments in thef ramebuffer results in a "
                             "nonrednerable target");
        }
        else
        {
            opengl_debug_log("Framebuffer", "Attach success!\n");
        }
        return *this;
    }

#define IS_FIRST_FRAMEBUFFER (this == _M_set->_M_framebuffers.data())
    OpenGL_FrameBuffer& OpenGL_FrameBuffer::gen_framebuffer(const FrameBufferCreateInfo::Buffer& buffer,
                                                            const Size2D& size, const FrameBufferCreateInfo& info)
    {
        glGenFramebuffers(1, &_M_instance_id);
        glBindFramebuffer(GL_FRAMEBUFFER, _M_instance_id);

        ArrayIndex index = 0;
        Vector<GLenum> attachments(buffer.color_attachments.size(), 0);

        if (IS_FIRST_FRAMEBUFFER)
            _M_set->_M_clear_color_values.resize(buffer.color_attachments.size());

        for (auto& color_attachment : buffer.color_attachments)
        {
            opengl_debug_log("Framebuffer", "Attaching texture[%d] to buffer %p", (int) color_attachment.texture_id,
                             this);
            attach_texture(color_attachment, GL_COLOR_ATTACHMENT0 + index);
            attachments[index] = GL_COLOR_ATTACHMENT0 + index;

            if (IS_FIRST_FRAMEBUFFER && info.color_clear_data[index].clear_on_bind)
            {
                _M_set->_M_clear_color_values[index] = {
                        info.color_clear_data[index].clear_value.color.r,
                        info.color_clear_data[index].clear_value.color.g,
                        info.color_clear_data[index].clear_value.color.b,
                        info.color_clear_data[index].clear_value.color.a,
                };

                _M_set->_M_command_buffer.next(new OpenGL_Command(
                        glClearBufferfv, static_cast<GLenum>(GL_COLOR), static_cast<GLint>(index),
                        reinterpret_cast<const GLfloat*>(_M_set->_M_clear_color_values[index].data())));
            }
            index++;
        }

        if (!attachments.empty())
        {
            glDrawBuffers(attachments.size(), attachments.data());
        }

        if (buffer.depth_stencil_attachment.has_value())
        {
            auto& depth_attachment = buffer.depth_stencil_attachment.value();
            attach_texture(depth_attachment,
                           get_attachment_type(GET_TYPE(OpenGL_Texture, depth_attachment.texture_id)->_M_pixel_type));

            if (IS_FIRST_FRAMEBUFFER && info.depth_stencil_clear_data.clear_on_bind)
            {
                _M_set->_M_command_buffer.next(new OpenGL_Command(
                        glClearBufferfi, static_cast<GLenum>(GL_DEPTH_STENCIL), static_cast<GLint>(0),
                        static_cast<GLfloat>(info.depth_stencil_clear_data.clear_value.depth_stencil.depth),
                        static_cast<GLint>(info.depth_stencil_clear_data.clear_value.depth_stencil.stencil)));
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return *this;
    }

    OpenGL_FrameBufferSet& OpenGL_FrameBufferSet::gen_framebuffer(const FrameBufferCreateInfo& info)
    {
        _M_framebuffers.resize(info.buffers.size());

        size_t index = 0;

        for (auto& buffer : _M_framebuffers)
        {
            buffer._M_set = this;
            buffer.gen_framebuffer(info.buffers[index++], info.size, info);
        }
        _M_viewport.pos  = {0, 0};
        _M_viewport.size = info.size;
        _M_scissor.pos   = {0, 0};
        _M_scissor.size  = info.size;
        return *this;
    }

    OpenGL_FrameBufferSet& OpenGL_FrameBufferSet::bind_framebuffer(size_t buffer_index)
    {
        API->_M_current_framebuffer = this;
        glBindFramebuffer(GL_FRAMEBUFFER, _M_framebuffers[buffer_index]._M_instance_id);
        _M_command_buffer.apply();
        return update_viewport().update_scissor();
    }

    OpenGL_FrameBufferSet& OpenGL_FrameBufferSet::framebuffer_viewport(const ViewPort& viewport)
    {
        _M_viewport = viewport;
        if (API->_M_current_framebuffer == this)
            update_viewport();
        return *this;
    }

    OpenGL_FrameBufferSet& OpenGL_FrameBufferSet::framebuffer_scissor(const Scissor& scissor)
    {
        _M_scissor = scissor;
        if (API->_M_current_framebuffer == this)
            update_scissor();
        return *this;
    }

    OpenGL_FrameBufferSet& OpenGL_FrameBufferSet::update_viewport()
    {
        glViewport(_M_viewport.pos.x, _M_viewport.pos.y, _M_viewport.size.x, _M_viewport.size.y);
        glDepthRangef(_M_viewport.min_depth, _M_viewport.max_depth);

        return *this;
    }

    OpenGL_FrameBufferSet& OpenGL_FrameBufferSet::update_scissor()
    {
        glScissor(_M_scissor.pos.x, _M_scissor.pos.y, _M_scissor.size.x, _M_scissor.size.y);
        return *this;
    }


    OpenGL_FrameBuffer::~OpenGL_FrameBuffer()
    {
        if (_M_instance_id)
        {
            glDeleteFramebuffers(1, &_M_instance_id);
        }
    }
}// namespace Engine
