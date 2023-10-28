#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class TextureType
    {
        Texture2D,
        TextureCubeMap,
    };


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

    enum class VertexAttributeInputRate : byte
    {
        Vertex   = 0,
        Instance = 1
    };

#define make_shader_data_type(stride, color, id)                                                                       \
    ((static_cast<EnumerateType>(stride) << 25) | (static_cast<EnumerateType>(color) << 18) |                          \
     static_cast<EnumerateType>(id))

    enum class ShaderDataType : EnumerateType
    {
        Bool  = make_shader_data_type(1, 0, 0),
        Int   = make_shader_data_type(4, 20, 1),
        UInt  = make_shader_data_type(4, 19, 2),
        Float = make_shader_data_type(4, 18, 3),
        Vec2  = make_shader_data_type(8, 39, 4),
        Vec3  = make_shader_data_type(12, 64, 5),
        Vec4  = make_shader_data_type(16, 88, 6),
        IVec2 = make_shader_data_type(8, 38, 7),
        IVec3 = make_shader_data_type(12, 63, 8),
        IVec4 = make_shader_data_type(16, 87, 9),
        UVec2 = make_shader_data_type(8, 37, 10),
        UVec3 = make_shader_data_type(12, 62, 11),
        UVec4 = make_shader_data_type(16, 86, 12),
        BVec2 = make_shader_data_type(8, 0, 13),
        BVec3 = make_shader_data_type(12, 0, 14),
        BVec4 = make_shader_data_type(16, 0, 15),
        Mat2  = make_shader_data_type(32, 0, 16),
        Mat3  = make_shader_data_type(48, 0, 17),
        Mat4  = make_shader_data_type(64, 0, 18)

    };
#undef make_shader_data_type
}// namespace Engine
