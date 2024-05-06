#pragma once
#include <Graphics/render_target.hpp>

namespace Engine
{
    class ENGINE_EXPORT DepthRenderTarget : public RenderTarget
    {
        declare_class(DepthRenderTarget, RenderTarget);

    public:
        DepthRenderTarget();
        DepthRenderTarget& rhi_create() override;
    };
}
