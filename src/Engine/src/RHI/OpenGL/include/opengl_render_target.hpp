#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_RenderTarget : public RHI_RenderTarget {
        ViewPort m_viewport;
        Scissor m_scissor;
        GLuint m_framebuffer            = 0;
        bool m_has_depth_stencil_buffer = false;

        void bind() override;
        void viewport(const ViewPort& viewport) override;
        void scissor(const Scissor& scissor) override;
        void clear_depth_stencil(const DepthStencilClearValue& value) override;
        void clear_color(const ColorClearValue& color, byte layout) override;


        bool is_active() const;

        void update_viewport();
        void update_scissors();

        OpenGL_RenderTarget& init(const class RenderTarget* render_target);
        OpenGL_RenderTarget& attach_texture(const class Texture2D* texture_attachmend, GLuint attachment);

        ~OpenGL_RenderTarget();
    };

    struct OpenGL_MainRenderTarget : OpenGL_RenderTarget {
        OpenGL_MainRenderTarget();

        void bind() override;
        ~OpenGL_MainRenderTarget();
    };
}// namespace Engine
