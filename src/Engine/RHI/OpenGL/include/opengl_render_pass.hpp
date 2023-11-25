#pragma once
#include <Graphics/rhi.hpp>


namespace Engine
{
    struct OpenGL_RenderPass : public RHI_RenderPass {
        Vector<bool> _M_clear_color_attachmend_on_bind;
        bool _M_clear_depth_stencil;
        bool _M_has_depth_stencil_attachment = false;

        void apply(struct OpenGL_RenderTarget* render_target);
    };

    struct OpenGL_MainRenderPass : OpenGL_RenderPass {
        OpenGL_MainRenderPass();

        bool is_destroyable() const override;
    };


}// namespace Engine
