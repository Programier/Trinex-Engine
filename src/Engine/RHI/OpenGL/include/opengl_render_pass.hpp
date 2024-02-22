#pragma once
#include <Graphics/rhi.hpp>


namespace Engine
{
    struct OpenGL_RenderPass : public RHI_RenderPass {
        Vector<bool> m_clear_color_attachmend_on_bind;
        bool m_clear_depth_stencil;
        bool m_has_depth_stencil_attachment = false;
        void apply(struct OpenGL_RenderTarget* render_target);

        ~OpenGL_RenderPass();
    };

    struct OpenGL_MainRenderPass : OpenGL_RenderPass {
        bool is_destroyable() const override;
    };


}// namespace Engine
