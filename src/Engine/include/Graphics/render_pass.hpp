#pragma once
#include <Core/api_object.hpp>
#include <Core/enums.hpp>

namespace Engine
{
    class Texture2D;

    class ENGINE_EXPORT RenderPass : public ApiObject
    {
        declare_class(RenderPass, ApiObject);

    public:
        enum Type : byte
        {
            Window,
            GBuffer,
            __COUNT__
        };

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

        static RenderPass* default_pass(Type type);
    };
}// namespace Engine
