#pragma once
#include <Core/implement.hpp>
#include <Graphics/render_target_base.hpp>


namespace Engine
{
    class RenderTargetTexture;

    class ENGINE_EXPORT RenderTarget : public RenderTargetBase
    {
        declare_class(RenderTarget, RenderTargetBase);

    public:
        struct ENGINE_EXPORT ColorAttachemnt {
            Pointer<RenderTargetTexture> texture;
            ColorClearValue color_clear;
        };

        struct ENGINE_EXPORT DepthAttachemnt {
            Pointer<RenderTargetTexture> texture;
            DepthStencilClearValue depth_stencil_clear;
        };

        Vector<ColorAttachemnt> color_attachments;
        DepthAttachemnt depth_stencil_attachment;

        Size2D size;

        RenderTarget();
        RenderTarget& rhi_create() override;
        Size2D render_target_size() const override;
        ~RenderTarget() override;
    };
}// namespace Engine
