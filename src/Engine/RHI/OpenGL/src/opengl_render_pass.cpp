#include <Graphics/render_pass.hpp>
#include <opengl_api.hpp>
#include <opengl_render_pass.hpp>
#include <opengl_render_target.hpp>

namespace Engine
{
    void OpenGL_RenderPass::apply(OpenGL_RenderTarget* render_target)
    {
        for (size_t i = 0, count = _M_clear_color_attachmend_on_bind.size(); i < count; i++)
        {
            if (_M_clear_color_attachmend_on_bind[i])
            {
                const ColorClearValue& color = render_target->_M_clear_color[i];
                glClearBufferfv(GL_COLOR, static_cast<GLint>(i), &color.x);
            }
        }

        if (_M_has_depth_stencil_attachment && _M_clear_depth_stencil)
        {
            glClearBufferfi(GL_DEPTH_STENCIL, 0, render_target->_M_depth_stencil_clear.depth,
                            render_target->_M_depth_stencil_clear.stencil);
        }
    }

    RHI_RenderPass* OpenGL::create_render_pass(const RenderPass* render_pass)
    {
        OpenGL_RenderPass* pass               = new OpenGL_RenderPass();
        pass->_M_has_depth_stencil_attachment = render_pass->has_depth_stancil;
        pass->_M_clear_color_attachmend_on_bind.reserve(render_pass->color_attachments.size());

        for (auto& ell : render_pass->color_attachments)
        {
            pass->_M_clear_color_attachmend_on_bind.push_back(ell.clear_on_bind);
        }

        pass->_M_clear_depth_stencil = render_pass->depth_stencil_attachment.clear_on_bind;
        return pass;
    }

    bool OpenGL_MainRenderPass::is_destroyable() const
    {
        return false;
    }
}// namespace Engine
