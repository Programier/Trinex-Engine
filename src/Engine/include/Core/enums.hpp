#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class TextureType
    {
        Texture2D,
        TextureCubeMap,
    };

    enum class ColorFormatMetaData
    {
        None    = 0,
        Unorm   = 1,
        Snorm   = 2,
        Uscaled = 3,
        Sscaled = 4,
        Uint    = 5,
        Sint    = 6,
        Srgb    = 7,
        Sfloat  = 8
    };

    enum class ColorFormatAspect
    {
        None         = 0,
        Color        = 1,
        Depth        = 2,
        Stencil      = 3,
        DepthStencil = 4,
    };

#define color_shift(a, b) (static_cast<EnumerateType>(a) << b)
#define make_color_format(count, size, metadata, aspect, padding)                                                      \
    (color_shift(count, 0) | color_shift(size / 8, 3) | color_shift(ColorFormatMetaData::metadata, 6) |                \
     color_shift(ColorFormatAspect::aspect, 10) | color_shift(padding, 13))

    enum class ColorFormat : EnumerateType
    {
        Undefined = 0,
        R8Unorm   = make_color_format(1, 8, Unorm, Color, 0),
        R8Snorm   = make_color_format(1, 8, Snorm, Color, 0),
        R8Uscaled = make_color_format(1, 8, Uscaled, Color, 0),
        R8Sscaled = make_color_format(1, 8, Sscaled, Color, 0),
        S8Uint    = make_color_format(1, 8, Uint, Stencil, 0),
        R8Uint    = make_color_format(1, 8, Uint, Color, 1),
        R8Sint    = make_color_format(1, 8, Sint, Color, 0),
        R8Srgb    = make_color_format(1, 8, Srgb, Color, 0),

        R16Unorm   = make_color_format(1, 16, Unorm, Color, 0),
        D16Unorm   = make_color_format(1, 16, Unorm, Color, 1),
        R16Snorm   = make_color_format(1, 16, Snorm, Color, 0),
        R16Uscaled = make_color_format(1, 16, Uscaled, Color, 0),
        R16Sscaled = make_color_format(1, 16, Sscaled, Color, 0),
        R16Uint    = make_color_format(1, 16, Uint, Color, 0),
        R16Sint    = make_color_format(1, 16, Sint, Color, 0),
        R16Sfloat  = make_color_format(1, 16, Sfloat, Color, 0),

        D32Sfloat = make_color_format(1, 32, Sfloat, Depth, 0),
        R32Sfloat = make_color_format(1, 32, Sfloat, Color, 1),
        R32Uint   = make_color_format(1, 32, Uint, Color, 0),
        R32Sint   = make_color_format(1, 32, Sint, Color, 0),

        R8G8Unorm   = make_color_format(2, 8, Unorm, Color, 0),
        R8G8Snorm   = make_color_format(2, 8, Snorm, Color, 0),
        R8G8Uscaled = make_color_format(2, 8, Uscaled, Color, 0),
        R8G8Sscaled = make_color_format(2, 8, Sscaled, Color, 0),
        R8G8Uint    = make_color_format(2, 8, Uint, Color, 0),
        R8G8Sint    = make_color_format(2, 8, Sint, Color, 0),
        R8G8Srgb    = make_color_format(2, 8, Srgb, Color, 0),

        R16G16Unorm   = make_color_format(2, 16, Unorm, Color, 0),
        R16G16Snorm   = make_color_format(2, 16, Snorm, Color, 0),
        R16G16Uscaled = make_color_format(2, 16, Uscaled, Color, 0),
        R16G16Sscaled = make_color_format(2, 16, Sscaled, Color, 0),
        R16G16Uint    = make_color_format(2, 16, Uint, Color, 0),
        R16G16Sint    = make_color_format(2, 16, Sint, Color, 0),
        R16G16Sfloat  = make_color_format(2, 16, Sfloat, Color, 0),

        D16UnormS8Uint = make_color_format(2, 24, Unorm, DepthStencil, 0),

        D24UnormS8Uint = make_color_format(2, 32, Unorm, DepthStencil, 1),
        R32G32Uint     = make_color_format(2, 32, Uint, Color, 0),
        R32G32Sint     = make_color_format(2, 32, Sint, Color, 0),
        R32G32Sfloat   = make_color_format(2, 32, Sfloat, Color, 0),

        D32SfloatS8Uint = make_color_format(2, 40, Sfloat, DepthStencil, 0),

        R8G8B8Unorm   = make_color_format(3, 8, Unorm, Color, 0),
        B8G8R8Unorm   = make_color_format(3, 8, Unorm, Color, 1),
        R8G8B8Snorm   = make_color_format(3, 8, Snorm, Color, 0),
        B8G8R8Snorm   = make_color_format(3, 8, Snorm, Color, 1),
        R8G8B8Uscaled = make_color_format(3, 8, Uscaled, Color, 0),
        B8G8R8Uscaled = make_color_format(3, 8, Uscaled, Color, 1),
        R8G8B8Sscaled = make_color_format(3, 8, Sscaled, Color, 0),
        B8G8R8Sscaled = make_color_format(3, 8, Sscaled, Color, 1),
        R8G8B8Uint    = make_color_format(3, 8, Uint, Color, 0),
        B8G8R8Uint    = make_color_format(3, 8, Uint, Color, 1),
        R8G8B8Sint    = make_color_format(3, 8, Sint, Color, 0),
        B8G8R8Sint    = make_color_format(3, 8, Sint, Color, 1),
        R8G8B8Srgb    = make_color_format(3, 8, Srgb, Color, 0),
        B8G8R8Srgb    = make_color_format(3, 8, Srgb, Color, 1),

        R16G16B16Unorm   = make_color_format(3, 16, Unorm, Color, 0),
        R16G16B16Snorm   = make_color_format(3, 16, Snorm, Color, 0),
        R16G16B16Uscaled = make_color_format(3, 16, Uscaled, Color, 0),
        R16G16B16Sscaled = make_color_format(3, 16, Sscaled, Color, 0),
        R16G16B16Uint    = make_color_format(3, 16, Uint, Color, 0),
        R16G16B16Sint    = make_color_format(3, 16, Sint, Color, 0),
        R16G16B16Sfloat  = make_color_format(3, 16, Sfloat, Color, 0),

        R32G32B32Uint   = make_color_format(3, 32, Uint, Color, 0),
        R32G32B32Sint   = make_color_format(3, 32, Sint, Color, 0),
        R32G32B32Sfloat = make_color_format(3, 32, Sfloat, Color, 0),
        R8G8B8A8Unorm   = make_color_format(4, 8, Unorm, Color, 0),
        B8G8R8A8Unorm   = make_color_format(4, 8, Unorm, Color, 1),
        R8G8B8A8Snorm   = make_color_format(4, 8, Snorm, Color, 0),
        B8G8R8A8Snorm   = make_color_format(4, 8, Snorm, Color, 1),
        R8G8B8A8Uscaled = make_color_format(4, 8, Uscaled, Color, 0),
        B8G8R8A8Uscaled = make_color_format(4, 8, Uscaled, Color, 1),
        R8G8B8A8Sscaled = make_color_format(4, 8, Sscaled, Color, 0),
        B8G8R8A8Sscaled = make_color_format(4, 8, Sscaled, Color, 1),
        R8G8B8A8Uint    = make_color_format(4, 8, Uint, Color, 0),
        B8G8R8A8Uint    = make_color_format(4, 8, Uint, Color, 1),
        R8G8B8A8Sint    = make_color_format(4, 8, Sint, Color, 0),
        B8G8R8A8Sint    = make_color_format(4, 8, Sint, Color, 1),
        R8G8B8A8Srgb    = make_color_format(4, 8, Srgb, Color, 0),
        B8G8R8A8Srgb    = make_color_format(4, 8, Srgb, Color, 1),

        R16G16B16A16Unorm   = make_color_format(4, 16, Unorm, Color, 0),
        R16G16B16A16Snorm   = make_color_format(4, 16, Snorm, Color, 0),
        R16G16B16A16Uscaled = make_color_format(4, 16, Uscaled, Color, 0),
        R16G16B16A16Sscaled = make_color_format(4, 16, Sscaled, Color, 0),
        R16G16B16A16Uint    = make_color_format(4, 16, Uint, Color, 0),
        R16G16B16A16Sint    = make_color_format(4, 16, Sint, Color, 0),
        R16G16B16A16Sfloat  = make_color_format(4, 16, Sfloat, Color, 0),

        R32G32B32A32Uint   = make_color_format(4, 32, Uint, Color, 0),
        R32G32B32A32Sint   = make_color_format(4, 32, Sint, Color, 0),
        R32G32B32A32Sfloat = make_color_format(4, 32, Sfloat, Color, 0),
    };

#undef make_color_format
#undef color_shift


    enum class CompareMode
    {
        None,
        RefToTexture
    };

    enum class SamplerFilter : EnumerateType
    {
        Point     = 0,
        Bilinear  = 1,
        Trilinear = 2,
    };

    enum class SwizzleValue : EnumerateType
    {
        Identity = 0,
        Zero     = 1,
        One      = 2,
        R        = 3,
        G        = 4,
        B        = 5,
        A        = 6
    };

    enum class WrapValue : EnumerateType
    {
        Repeat            = 0,
        ClampToEdge       = 1,
        ClampToBorder     = 2,
        MirroredRepeat    = 3,
        MirrorClampToEdge = 4,
    };

    enum class TextureCubeMapFace : byte
    {
        Front = 0,
        Back  = 1,
        Up    = 2,
        Down  = 3,
        Left  = 4,
        Right = 5
    };

    enum class IndexBufferComponent : EnumerateType
    {
        UnsignedByte  = 0,
        UnsignedInt   = 1,
        UnsignedShort = 2,
    };

    enum class VertexBufferSemantic : EnumerateType
    {
        Position     = 0,
        TexCoord     = 1,
        Color        = 2,
        Normal       = 3,
        Tangent      = 4,
        Binormal     = 5,
        BlendWeight  = 6,
        BlendIndices = 7,
    };

    enum class Coord
    {
        X,
        Y,
        Z
    };

    enum class EngineAPI : EnumerateType
    {
        NoAPI  = 0,
        OpenGL = 1,
        Vulkan = 2
    };


    enum class DataType : EnumerateType
    {
        Text = 0,
        Binary
    };


    enum class SystemName
    {
        LinuxOS,
        WindowsOS,
        AndroidOS
    };

    enum class ColorComponent : EnumerateType
    {
        R = 1,
        G = 2,
        B = 4,
        A = 8,
    };

    enum class CompareFunc : EnumerateType
    {
        Always   = 0,
        Lequal   = 1,
        Gequal   = 2,
        Less     = 3,
        Greater  = 4,
        Equal    = 5,
        NotEqual = 6,
        Never    = 7,
    };

    enum class PhysicalSizeMetric
    {
        Inch,
        Сentimeters,
    };

    enum class AccessType
    {
        Private,
        Protected,
        Public
    };

    enum class StencilOp : EnumerateType
    {
        Keep     = 0,
        Zero     = 1,
        Replace  = 2,
        Incr     = 3,
        IncrWrap = 4,
        Decr     = 5,
        DecrWrap = 6,
        Invert   = 7,
    };

    enum class BlendFunc : EnumerateType
    {
        Zero                  = 0,
        One                   = 1,
        SrcColor              = 2,
        OneMinusSrcColor      = 3,
        DstColor              = 4,
        OneMinusDstColor      = 5,
        SrcAlpha              = 6,
        OneMinusSrcAlpha      = 7,
        DstAlpha              = 8,
        OneMinusDstAlpha      = 9,
        ConstantColor         = 10,
        OneMinusConstantColor = 11,
        ConstantAlpha         = 12,
        OneMinusConstantAlpha = 13,
    };

    enum class BlendOp : EnumerateType
    {
        Add             = 0,
        Subtract        = 1,
        ReverseSubtract = 2,
        Min             = 3,
        Max             = 4,
    };

    enum class Primitive : EnumerateType
    {
        Triangle = 0,
        Line     = 1,
        Point    = 2,
    };

    enum class EnableCap : EnumerateType
    {
        Blend       = 0,
        DepthTest   = 1,
        CullFace    = 2,
        StencilTest = 3,
    };

    using DepthFunc = CompareFunc;

    union BlendConstants
    {
        Array<float, 4> array = {0.0f, 0.0f, 0.0f, 0.0f};
        Vector4D vector;
    };

    enum class PrimitiveTopology : EnumerateType
    {
        TriangleList               = 0,
        PointList                  = 1,
        LineList                   = 2,
        LineStrip                  = 3,
        TriangleStrip              = 4,
        TriangleFan                = 5,
        LineListWithAdjacency      = 6,
        LineStripWithAdjacency     = 4,
        TriangleListWithAdjacency  = 8,
        TriangleStripWithAdjacency = 9,
        PatchList                  = 10,
    };

    enum class PolygonMode : EnumerateType
    {
        Fill  = 0,
        Line  = 1,
        Point = 2,
    };

    enum class CullMode : EnumerateType
    {
        None         = 0,
        Front        = 1,
        Back         = 2,
        FrontAndBack = 3,
    };

    enum class FrontFace : EnumerateType
    {
        ClockWise        = 0,
        CounterClockWise = 1,
    };

    enum class LogicOp : EnumerateType
    {
        Clear        = 0,
        And          = 1,
        AndReverse   = 2,
        Copy         = 3,
        AndInverted  = 4,
        NoOp         = 5,
        Xor          = 6,
        Or           = 7,
        Nor          = 8,
        Equivalent   = 10,
        Invert       = 11,
        OrReverse    = 12,
        CopyInverted = 13,
        OrInverted   = 14,
        Nand         = 15,
        Set          = 16,
    };

    enum class WindowAttribute : EnumerateType
    {
        None                   = 0,
        Resizable              = 1,
        FullScreen             = 2,
        FullScreenDesktop      = 3,
        Shown                  = 4,
        Hidden                 = 5,
        BorderLess             = 6,
        MouseFocus             = 7,
        InputFocus             = 8,
        InputGrabbed           = 9,
        Minimized              = 10,
        Maximized              = 11,
        TransparentFramebuffer = 12,
        MouseCapture           = 13,
        AllowHighDPI           = 14,
        MouseGrabbed           = 15,
        KeyboardGrabbed        = 16,
    };

    enum class CursorMode : EnumerateType
    {
        Normal,
        Hidden
    };

    enum class WindowOrientation : EnumerateType
    {
        Landscape        = 1,
        LandscapeFlipped = 2,
        Portrait         = 3,
        PortraitFlipped  = 4
    };

    enum class MessageBoxType
    {
        Error,
        Warning,
        Info,
    };
}// namespace Engine
