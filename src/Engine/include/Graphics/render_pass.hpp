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
        struct Attachment {
            ColorFormat format;
            MipMapLevel mip_level = 0;
            bool clear_on_bind    = true;
        };

        Vector<Attachment> color_attachments;
        Attachment depth_stencil_attachment;
        bool has_depth_stancil = false;
    };
}// namespace Engine
