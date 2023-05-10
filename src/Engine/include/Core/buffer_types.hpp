#pragma once
#include <Core/engine_types.hpp>
#include <optional>


namespace Engine
{
    enum class IndexBufferComponent : EnumerateType
    {
        UnsignedByte  = 0,
        UnsignedInt   = 1,
        UnsignedShort = 2,
    };


    using BufferType = size_t;

    struct ViewPort {
        Point2D pos = {0.0f, 0.0f};
        Size2D size;
        float min_depth = 0.0f;
        float max_depth = 1.0f;
    };

    struct Scissor {
        Point2D pos = {0.0f, 0.0f};
        Size2D size;
    };

    IndexBufferComponent index_buffer_component_of(const std::type_info& info);
    using FrameBufferOutputLocation = byte;


    using ColorClearValue = Vector4D;

    struct DepthStencilClearValue {
        float depth  = 1.0;
        byte stencil = 0.0;
    };

    union FrameBufferClearValue
    {
        ColorClearValue color;
        DepthStencilClearValue depth_stencil;

        FrameBufferClearValue() : color(0.0f, 0.0f, 0.0f, 1.0f)
        {}
    };

    struct FrameBufferAttachment {
        Identifier texture_id       = 0;
        MipMapLevel mip_level  = 0;
    };

    struct FrameBufferAttachmentClearData
    {
        byte clear_on_bind : 1 = 1;
        FrameBufferClearValue clear_value;
    };

    struct FrameBufferCreateInfo {
        struct Buffer {
            Vector<FrameBufferAttachment> color_attachments;
            std::optional<FrameBufferAttachment> depth_stencil_attachment;
        };

        Vector<Buffer> buffers;
        Vector<FrameBufferAttachmentClearData> color_clear_data;
        FrameBufferAttachmentClearData depth_stencil_clear_data;
        Size2D size;
    };

    enum class VertexBufferSemantic : EnumerateType
    {
        Vertex       = 0,
        TexCoord     = 1,
        Color        = 2,
        Normal       = 3,
        Tangent      = 4,
        Binormal     = 5,
        BlendWeight  = 6,
        BlendIndices = 7,
    };
}// namespace Engine
