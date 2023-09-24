#pragma once
#include <Core/implement.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class Texture2D;

    class ENGINE_EXPORT RenderTarget : public BasicRenderTarget
    {
        declare_class(RenderTarget, RenderTarget);
    public:
        struct Attachment
        {
            Texture2D* texture  = nullptr;
            MipMapLevel mip_level = 0;
        };

        union ClearValue
        {
            ColorClearValue color;
            DepthStencilClearValue depth_stencil;

            ClearValue() : color(0.0f, 0.0f, 0.0f, 1.0f)
            {}
        };

        struct AttachmentClearData {
            byte clear_on_bind : 1 = 1;
            ClearValue clear_value;
        };

        Vector<Attachment> color_attachments;
        Attachment depth_stencil_attachment;
        Vector<AttachmentClearData> color_clear_data;
        AttachmentClearData depth_stencil_clear_data;
        Size2D size;

        RenderTarget& rhi_create() override;
    };
}// namespace Engine
