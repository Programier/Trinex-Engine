#pragma once
#include <Core/render_resource.hpp>
#include <Core/enums.hpp>
#include <Core/color_format.hpp>

namespace Engine
{
    class Texture2D;

    class ENGINE_EXPORT RenderPass : public RenderResource
    {
        declare_class(RenderPass, RenderResource);


    protected:
        static Vector<RenderPass*> _M_default_render_passes;

    public:
        struct Attachment {
            ColorFormat format;
            MipMapLevel mip_level = 0;
            bool clear_on_bind    = true;
        };

        Vector<Attachment> color_attachments;
        Attachment depth_stencil_attachment;
        bool has_depth_stancil = false;

        RenderPass& rhi_create() override;
    };
}// namespace Engine
