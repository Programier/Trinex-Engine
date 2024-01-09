#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class TextureType : EnumerateType
    {
        Texture2D,
        TextureCubeMap,
    };

    enum class CompareMode : EnumerateType
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

    enum class Swizzle : EnumerateType
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

    using DepthFunc = CompareFunc;

    enum class PrimitiveTopology : EnumerateType
    {
        TriangleList               = 0,
        PointList                  = 1,
        LineList                   = 2,
        LineStrip                  = 3,
        TriangleStrip              = 4,
        TriangleFan                = 5,
        LineListWithAdjacency      = 6,
        LineStripWithAdjacency     = 7,
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

    enum class VertexAttributeInputRate : byte
    {
        Vertex   = 0,
        Instance = 1
    };

    enum class ColorComponentMask : EnumerateType
    {
        RGBA = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::G, ColorComponent::B, ColorComponent::A),
        RGB  = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::G, ColorComponent::B),
        RGA  = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::G, ColorComponent::A),
        RG   = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::G),
        RBA  = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::B, ColorComponent::A),
        RB   = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::B),
        RA   = mask_of<EnumerateType>(ColorComponent::R, ColorComponent::A),
        R    = mask_of<EnumerateType>(ColorComponent::R),
        GBA  = mask_of<EnumerateType>(ColorComponent::G, ColorComponent::B, ColorComponent::A),
        GB   = mask_of<EnumerateType>(ColorComponent::G, ColorComponent::B),
        GA   = mask_of<EnumerateType>(ColorComponent::G, ColorComponent::A),
        G    = mask_of<EnumerateType>(ColorComponent::G),
        BA   = mask_of<EnumerateType>(ColorComponent::B, ColorComponent::A),
        B    = mask_of<EnumerateType>(ColorComponent::B),
        A    = mask_of<EnumerateType>(ColorComponent::A)
    };
}// namespace Engine
