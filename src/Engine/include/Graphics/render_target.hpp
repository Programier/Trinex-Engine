#pragma once
#include <Core/implement.hpp>
#include <Graphics/basic_render_target.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class Texture2D;

    class ENGINE_EXPORT RenderTarget : public BasicRenderTarget
    {
        declare_class(RenderTarget, RenderTarget);

    public:
        struct Attachment {
            Texture2D* texture    = nullptr;
            MipMapLevel mip_level = 0;
        };

        Vector<Attachment> color_attachments;
        Attachment depth_stencil_attachment;
        Vector<ColorClearValue> color_clear;
        DepthStencilClearValue depth_stencil_clear;
        Size2D size;

        RenderTarget& rhi_create();
    };
}// namespace Engine
