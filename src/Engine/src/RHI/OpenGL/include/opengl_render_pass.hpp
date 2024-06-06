#pragma once
#include <Graphics/rhi.hpp>


namespace Engine
{
    struct OpenGL_RenderPass : public RHI_RenderPass {
    };

    struct OpenGL_MainRenderPass : OpenGL_RenderPass {
        bool is_destroyable() const override;
    };
}// namespace Engine
