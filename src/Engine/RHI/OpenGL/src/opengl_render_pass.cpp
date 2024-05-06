#include <Graphics/render_pass.hpp>
#include <opengl_api.hpp>
#include <opengl_render_pass.hpp>
#include <opengl_render_target.hpp>

namespace Engine
{
    void OpenGL_RenderPass::apply(OpenGL_RenderTarget* render_target)
    {
        for (size_t i = 0, count = m_clear_color_attachmend_on_bind.size(); i < count; i++)
        {
            if (m_clear_color_attachmend_on_bind[i])
            {
                const ColorClearValue& color = render_target->m_clear_color[i];
                glClearBufferfv(GL_COLOR, static_cast<GLint>(i), &color.x);
            }
        }

        if (m_has_depth_stencil_attachment && m_clear_depth_stencil)
        {
            glDepthMask(GL_TRUE);
            glStencilMask(0xFFFFFFFF);
            glClearBufferfi(GL_DEPTH_STENCIL, 0, render_target->m_depth_stencil_clear.depth,
                            render_target->m_depth_stencil_clear.stencil);
        }
    }

    RHI_RenderPass* OpenGL::create_render_pass(const RenderPass* render_pass)
    {
        OpenGL_RenderPass* pass               = new OpenGL_RenderPass();
        pass->m_has_depth_stencil_attachment = render_pass->has_depth_stancil;
        pass->m_clear_color_attachmend_on_bind.reserve(render_pass->color_attachments.size());

        for (auto& ell : render_pass->color_attachments)
        {
            pass->m_clear_color_attachmend_on_bind.push_back(ell.clear_on_bind);
        }

        pass->m_clear_depth_stencil = render_pass->depth_stencil_attachment.clear_on_bind;
        return pass;
    }

    OpenGL_RenderPass::~OpenGL_RenderPass()
    {}

    bool OpenGL_MainRenderPass::is_destroyable() const
    {
        return false;
    }

    RHI_RenderPass* OpenGL::window_render_pass(RenderPass* engine_render_pass)
    {
        engine_render_pass->color_attachments.resize(1);
        engine_render_pass->color_attachments[0].clear_on_bind = true;
        engine_render_pass->color_attachments[0].format        = ColorFormat::R8G8B8A8;
        return m_main_render_pass;
    }
}// namespace Engine
