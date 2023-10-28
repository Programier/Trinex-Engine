#pragma once
#include <Core/structures.hpp>
#include <optional>

namespace Engine
{
    struct RHI_FrameBuffer;
    struct RHI_Texture;

    struct FrameBufferAttachment {
        RHI_Texture* texture  = nullptr;
        MipMapLevel mip_level = 0;
    };

    union FrameBufferClearValue
    {
        ColorClearValue color;
        DepthStencilClearValue depth_stencil;

        FrameBufferClearValue() : color(0.0f, 0.0f, 0.0f, 1.0f)
        {}
    };

    struct FrameBufferAttachmentClearData {
        byte clear_on_bind : 1 = 1;
        FrameBufferClearValue clear_value;
    };
}// namespace Engine
