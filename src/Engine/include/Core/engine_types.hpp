#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>


namespace Engine
{
    using byte = std::uint8_t;
    using signed_byte = std::int8_t;
    using Vector1D = glm::vec1;

    using Point1D = float;
    using Offset1D = float;
    using Size1D = float;
    using Scale1D = float;
    using Translate1D = float;
    using EulerAngle1D = float;
    using Distance = float;

    using Point2D = glm::vec2;
    using Offset2D = glm::vec2;
    using Size2D = glm::vec2;
    using Scale2D = glm::vec2;
    using Translate2D = glm::vec2;
    using Vector2D = glm::vec2;
    using EulerAngle2D = glm::vec2;

    using Point3D = glm::vec3;
    using Offset3D = glm::vec3;
    using Size3D = glm::vec3;
    using Scale3D = glm::vec3;
    using Translate3D = glm::vec3;
    using Vector3D = glm::vec3;
    using EulerAngle3D = glm::vec3;
    using Force = glm::vec3;
    using LightColor = glm::vec3;

    using Matrix4f = glm::mat4;
    using Matrix3f = glm::mat3;
    using Matrix2f = glm::mat2;

    using Vector4D = glm::vec4;

    using ArrayIndex = std::size_t;
    using PriorityIndex = std::size_t;
    using Counter = std::size_t;

    using Quaternion = glm::quat;

    using TextureBindIndex = byte;

    using AssimpObject = const void*;
    using BitMask = std::size_t;
    using String = std::wstring;

    // Int Vectors
    using IntVector2D = glm::ivec2;
    using IntVector3D = glm::ivec3;
    using IntVector4D = glm::ivec4;

    using UIntVector2D = glm::uvec2;
    using UIntVector3D = glm::uvec3;
    using UIntVector4D = glm::uvec4;

    template<typename Type>
    class SizeLimits
    {
    public:
        Type min;
        Type max;

        SizeLimits() = default;
        SizeLimits(const Type& _min, const Type& _max) : min(_min), max(_max)
        {}

        Type& operator[](bool _max)
        {
            if (_max)
                return max;
            return min;
        }

        const Type& operator[](bool _max) const
        {
            if (_max)
                return max;
            return min;
        }
    };

    using SizeLimits1D = SizeLimits<Size1D>;
    using SizeLimits2D = SizeLimits<Size2D>;
    using SizeLimits3D = SizeLimits<Size3D>;


    using AABB_1D = SizeLimits1D;
    using AABB_2D = SizeLimits2D;
    using AABB_3D = SizeLimits3D;

    using ObjectID = uint32_t;
    using ObjID = std::uintptr_t;
    using MipMapLevel = std::uint32_t;
    using EnumerateType = std::uint32_t;

    using int_t = std::int32_t;
    using uint_t = std::uint32_t;

    enum class Coord
    {
        X,
        Y,
        Z
    };

    enum class CompareFunc
    {
        Lequal,
        Gequal,
        Less,
        Greater,
        Equal,
        NotEqual,
        Always,
        Never
    };

    enum class EngineAPI : EnumerateType
    {
        OpenGL = 0,
        Vulkan = 1
    };

    using AspectRation = glm::vec<2, std::int32_t, glm::defaultp>;

    enum WindowAttrib : uint16_t
    {
        WIN_NONE = 0,
        WIN_RESIZABLE = 1,
        WIN_FULLSCREEN = 2,
        WIN_FULLSCREEN_DESKTOP = 4,
        WIN_SHOWN = 8,
        WIN_HIDDEN = 16,
        WIN_BORDERLESS = 32,
        WIN_MOUSE_FOCUS = 64,
        WIN_INPUT_FOCUS = 128,
        WIN_INPUT_GRABBED = 256,
        WIN_MINIMIZED = 512,
        WIN_MAXIMIZED = 1024,
        WIN_TRANSPARENT_FRAMEBUFFER = 2048,
        WIN_MOUSE_CAPTURE = 4096,
        WIN_ALLOW_HIGHDPI = 8192,
        WIN_MOUSE_GRABBED = 16384,
        WIN_KEYBOARD_GRABBED = 32768
    };

    enum class CursorMode
    {
        NORMAL,
        HIDDEN
    };


    struct OpenGL_Version_S {
        int_t major;
        int_t minor;
    };

    enum WindowOrientation : uint_t
    {
        WIN_ORIENTATION_LANDSCAPE = 1,
        WIN_ORIENTATION_LANDSCAPE_FLIPPED = 2,
        WIN_ORIENTATION_PORTRAIT = 4,
        WIN_ORIENTATION_PORTRAIT_FLIPPED = 8
    };

    enum class TextureType
    {
        Texture_1D,
        Texture_1D_Array,
        Texture_2D,
        Texture_2D_Array,
        Texture_2D_MultiSample,
        Texture_2D_MultiSample_Array,
        Texture_3D,
        Texture_Rectangle,
        Texture_Cube_Map,
        Texture_Buffer
    };

    enum class PixelFormat : uint_t
    {
        DEPTH = 1,
        RGB = 3,
        RGBA = 4,
        RED = 5,
        GREEN = 6,
        BLUE = 7,
        ALPHA = 8,
        DEPTH_COMPONENT16 = 9,
        DEPTH_COMPONENT24 = 10,
        DEPTH_COMPONENT32F = 11,
        DEPTH24_STENCIL8 = 12,
        DEPTH32F_STENCIL8 = 13,
        STENCIL_INDEX8 = 14,
    };

    enum class BufferValueType : uint_t
    {
        FLOAT = 0,
        UNSIGNED_BYTE = 1,
        UNSIGNED_SHORT = 2,
        UNSIGNED_INT = 3,
        SHORT = 4,
        INT = 5,

        BYTE,
        HALF_FLOAT,
        UNSIGNED_SHORT_5_6_5,
        UNSIGNED_SHORT_4_4_4_4,
        UNSIGNED_SHORT_5_5_5_1,
        UNSIGNED_INT_2_10_10_10_REV,

        UNSIGNED_INT_24_8,
        UNSIGNED_INT_10F_11F_11F_REV,
        UNSIGNED_INT_5_9_9_9_REV,
        FLOAT_32_UNSIGNED_INT_24_8_REV,
    };

    enum class DepthStencilMode
    {
        DEPTH,
        STENCIL
    };

    enum class CompareMode
    {
        NONE,
        REF_TO_TEXTURE
    };

    enum class TextureFilter : uint_t
    {
        NEAREST = 0,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_NEAREST,
        LINEAR_MIPMAP_LINEAR
    };


    struct SwizzleRGBA {
        enum class SwizzleValue : uint_t
        {
            RED = 0,
            GREEN = 1,
            BLUE = 2,
            ALPHA = 3
        } R, G, B, A;
    };

    enum class WrapValue : uint_t
    {
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
        MIRRORED_REPEAT,
        REPEAT,
        MIRROR_CLAMP_TO_EDGE
    };

    struct TextureParams {
        TextureType type = TextureType::Texture_2D;
        PixelFormat format = PixelFormat::RGBA;
        BufferValueType pixel_type = BufferValueType::FLOAT;
        bool border = false;
    };

    enum class Primitive
    {
        POINT,
        LINE,
        TRIANGLE
    };

    enum class BufferUsage : byte
    {
        STATIC_DRAW = 0,
        STATIC_READ,
        STATIC_COPY,
        DYNAMIC_DRAW,
        DYNAMIC_READ,
        DYNAMIC_COPY,
        STREAM_DRAW,
        STREAM_READ,
        STREAM_COPY,
    };

    using DrawMode = BufferUsage;


    struct MeshAtribute {
        uint_t count;
        BufferValueType type;
        ArrayIndex offset = 0;
    };

    struct MeshInfo {
        std::vector<MeshAtribute> attributes;
        BufferUsage mode;
    };

    enum BufferBitType : uint_t
    {
        COLOR_BUFFER_BIT = 1,
        DEPTH_BUFFER_BIT = 2,
        STENCIL_BUFFER_BIT = 4,
    };

    using BufferType = std::size_t;

    enum class FrameBufferType : uint_t
    {
        FRAMEBUFFER = 0,
        DRAW_FRAMEBUFFER = 1,
        READ_FRAMEBUFFER = 2,
    };

    enum class FrameBufferAttach : uint_t
    {
        COLOR_ATTACHMENT = 0,
        DEPTH_ATTACHMENT = 1,
        STENCIL_ATTACHMENT = 2,
        DEPTH_STENCIL_ATTACHMENT = 3
    };

    enum class TextureCubeMapFace : byte
    {
        FRONT = 0,
        BACK = 1,
        UP = 2,
        DOWN = 3,
        LEFT = 4,
        RIGHT = 5
    };

    enum class EnableCap : uint_t
    {
        Blend = 0,
        DepthTest = 1,
        CullFace = 2,
        StencilTest = 3,
    };

    enum class BlendFunc : uint_t
    {
        Zero,
        One,
        SrcColor,
        OneMinusScrColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };


    enum class DataType
    {
        Text = 0,
        Binary
    };


    using ShaderSourceType = DataType;
    using FileBuffer = std::vector<byte>;

    struct ShaderParams {
        ShaderSourceType source_type;
        std::string name;
        FileBuffer vertex;
        FileBuffer fragment;
        FileBuffer compute;
        FileBuffer geometry;
    };

    enum class StencilOption : byte
    {
        KEEP,
        ZERO,
        REPLACE,
        INCR,
        INCR_WRAP,
        DECR,
        DECR_WRAP,
        INVERT,
    };

}// namespace Engine
