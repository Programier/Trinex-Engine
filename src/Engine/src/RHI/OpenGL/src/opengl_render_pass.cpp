#include <Graphics/render_pass.hpp>
#include <opengl_api.hpp>
#include <opengl_render_pass.hpp>
#include <opengl_render_target.hpp>

namespace Engine
{

    RHI_RenderPass* OpenGL::create_render_pass(const RenderPass* render_pass)
    {
        OpenGL_RenderPass* pass = new OpenGL_RenderPass();
        return pass;
    }

    bool OpenGL_MainRenderPass::is_destroyable() const
    {
        return false;
    }

    RHI_RenderPass* OpenGL::window_render_pass(RenderPass* engine_render_pass)
    {
        engine_render_pass->color_attachments.resize(1);
        engine_render_pass->color_attachments[0] = ColorFormat::R8G8B8A8;
        return m_main_render_pass;
    }
}// namespace Engine
