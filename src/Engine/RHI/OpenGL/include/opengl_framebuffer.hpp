#pragma once
#include <Graphics/rhi.hpp>
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

    struct OpenGL_FrameBufferSet : RHI_FrameBuffer {
        ViewPort _M_viewport;
        Scissor _M_scissor;
        OpenGL_CommandBuffer _M_command_buffer;
        Vector<Array<float, 4>> _M_clear_color_values;
        Vector<OpenGL_FrameBuffer> _M_framebuffers;

        OpenGL_FrameBufferSet(bool is_base = false);
        OpenGL_FrameBufferSet& framebuffer_scissor(const Scissor&);
        OpenGL_FrameBufferSet& gen_framebuffer(const FrameBufferCreateInfo& info);

        OpenGL_FrameBufferSet& update_viewport();
        OpenGL_FrameBufferSet& update_scissor();


        void bind(uint_t buffer_index) override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        void clear_color(const ColorClearValue& color, byte layout) override;
    };

    struct OpenGL_MainFrameBuffer : OpenGL_FrameBufferSet {
        OpenGL_MainFrameBuffer();
        void bind(uint_t buffer_index) override;
    };
}// namespace Engine
