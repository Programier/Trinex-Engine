#pragma once
#include <Core/buffer_types.hpp>
#include <opengl_command_buffer.hpp>
#include <opengl_object.hpp>

namespace Engine
{
    struct OpenGL_FrameBuffer : OpenGL_Object {
        implement_opengl_instance_hpp();
        struct OpenGL_FrameBufferSet* _M_set = nullptr;

        OpenGL_FrameBuffer();
        OpenGL_FrameBuffer& attach_texture(const FrameBufferAttachment& texture_attachment, GLenum attachment);
        OpenGL_FrameBuffer& gen_framebuffer(const FrameBufferCreateInfo::Buffer& buffer, const Size2D& size,
                                            const FrameBufferCreateInfo& info);
        OpenGL_FrameBuffer& bind();
        ~OpenGL_FrameBuffer();
    };

    struct OpenGL_FrameBufferSet : OpenGL_Object {
        ViewPort _M_viewport;
        Scissor _M_scissor;
        OpenGL_CommandBuffer _M_command_buffer;
        Vector<Array<float, 4>> _M_clear_color_values;
        Vector<OpenGL_FrameBuffer> _M_framebuffers;

        OpenGL_FrameBufferSet(bool is_base = false);
        OpenGL_FrameBufferSet& bind_framebuffer(size_t buffer_index);
        OpenGL_FrameBufferSet& framebuffer_viewport(const ViewPort&);
        OpenGL_FrameBufferSet& framebuffer_scissor(const Scissor&);
        OpenGL_FrameBufferSet& gen_framebuffer(const FrameBufferCreateInfo& info);

        OpenGL_FrameBufferSet& update_viewport();
        OpenGL_FrameBufferSet& update_scissor();
        implement_opengl_instance_hpp();
    };
}// namespace Engine
