#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>


namespace Engine
{
    using byte = std::uint8_t;
    using signed_byte = std::int8_t;
    using size_t = std::uint64_t;

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

    using ArrayIndex = size_t;
    using PriorityIndex = size_t;
    using Counter = size_t;


    using Quaternion = glm::quat;

    using TextureBindIndex = byte;
    using TextureAttachIndex = byte;

    using AssimpObject = const void*;
    using BitMask = size_t;
    using String = std::string;
#define STR(text) text

    using BoolVector2D = glm::bvec2;
    using BoolVector3D = glm::bvec3;
    using BoolVector4D = glm::bvec4;
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

    using DepthFunc = CompareFunc;

    enum class EngineAPI : EnumerateType
    {
        OpenGL = 0,
        Vulkan = 1
    };

    using AspectRation = glm::vec<2, std::int32_t, glm::defaultp>;

    enum WindowAttrib : uint16_t
    {
        WinNone = 0,
        WinResizable =  1,
        WinFullScreen =  2,
        WinFullScreenDesktop =  4,
        WinShown =  8,
        WinHidden =  16,
        WinBorderLess =  32,
        WinMouseFocus =  64,
        WinInputFocus =  128,
        WinInputGrabbed =  256,
        WinMinimized =  512,
        WinMaximized =  1024,
        WinTransparentFramebuffer =  2048,
        WinMouseCapture =  4096,
        WinAllowHighDPI =  8192,
        WinMouseGrabbed =  16384,
        WinKeyboardGrabbed =  32768,
    };

    enum class CursorMode
    {
        Normal,
        Hidden
    };


    struct OpenGL_Version_S {
        int_t major;
        int_t minor;
    };

    enum WindowOrientation : uint_t
    {
        WinOrientationLandscape =  1,
        WinOrientationLandscapeFlipped =  2,
        WinOrientationPortrait =  4,
        WinOrientationPortraitFlipped =  8

    };

    enum class TextureType
    {
        Texture2D,
        Texture3D,
        TextureCubeMap,
    };

    enum class PixelType : uint_t
    {
        Depth =  1,
        RGB =  3,
        RGBA =  4,
        Red =  5,
        DepthComponent16 =  9,
        DepthComponent24 =  10,
        DepthComponent32f =  11,
        Depth24Stencil8 =  12,
        Depth32fStencil8 =  13,
    };

    enum class PixelComponentType : uint_t
    {
        Float = 0,
        UnsignedByte= 1,
    };

    enum class IndexBufferComponent : uint_t
    {
        UnsignedInt,
        UnsignedShort,
        UnsignedByte
    };

    enum class ImageFormat : uint_t
    {
        R32Uint,
        VkFormatR32Uint,
        R8g8b8a8Srgb,
    };

    enum class BufferValueType : uint_t
    {
        Float = 0,
        UnsignedByte = 1,
        UnsignedShort = 2,
        UnsignedInt = 3,
        Short = 4,
        Int = 5,

        Byte,
        HalfFloat,
        UnsignedShort565,
        UnsignedShort4444,
        UnsignedShort5551,
        UnsignedInt2101010Rev,

        UnsignedInt248,
        UnsignedInt10f11f11fRev,
        UnsignedInt5999Rev,
        Float32UnsignedInt248Rev,
    };

    enum class DepthStencilMode
    {
        Depth,
        Stencil
    };

    enum class CompareMode
    {
        None,
        RefToTexture
    };

    enum class TextureFilter : uint_t
    {
        Nearest = 0,
        Linear,
        NearestMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapNearest,
        LinearMipmapLinear
    };


    struct SwizzleRGBA {
        enum class SwizzleValue : uint_t
        {
            Red = 0,
            Green = 1,
            Blue = 2,
            Alpha = 3
        } R, G, B, A;
    };

    enum class WrapValue : uint_t
    {
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat,
        Repeat,
        MirrorClampToEdge
    };


    struct TextureParams {
        TextureType type = TextureType::Texture2D;
        PixelType pixel_type = PixelType::RGBA;
        PixelComponentType pixel_component_type = PixelComponentType::Float;
    };

    enum class Primitive
    {
        Point,
        Line,
        Triangle
    };

    enum class BufferUsage : byte
    {
        StaticDraw = 0,
        StaticRead,
        StaticCopy,
        DynamicDraw,
        DynamicRead,
        DynamicCopy,
        StreamDraw,
        StreamRead,
        StreamCopy,
    };

    using DrawMode = BufferUsage;

    struct ShaderDataType {
        enum : uint_t
        {
            Bool = 0,
            Int = 1,
            UInt = 2,
            Float = 3,
            Vec2 = 4,
            Vec3 = 5,
            Vec4 = 6,
            IVec2 = 7,
            IVec3 = 8,
            IVec4 = 9,
            UVec2 = 10,
            UVec3 = 11,
            UVec4 = 12,
            BVec2 = 13,
            BVec3 = 14,
            BVec4 = 15,
            Mat2 = 16,
            Mat3 = 17,
            Mat4 = 18
        } type;

        size_t size = 0;
        size_t count = 1;

        template<class Type>
        inline static ShaderDataType type_of(size_t count = 1)
        {
            ShaderDataType result_type;
            result_type.size = sizeof(Type) * count;
            result_type.count = count;

            static const std::unordered_map<std::type_index, typeof(result_type.type)> types = {
                    {std::type_index(typeid(bool)), ShaderDataType::Bool},
                    {std::type_index(typeid(int_t)), ShaderDataType::Int},
                    {std::type_index(typeid(uint_t)), ShaderDataType::UInt},
                    {std::type_index(typeid(float)), ShaderDataType::Float},
                    {std::type_index(typeid(Vector2D)), ShaderDataType::Vec2},
                    {std::type_index(typeid(Vector3D)), ShaderDataType::Vec3},
                    {std::type_index(typeid(Vector4D)), ShaderDataType::Vec4},
                    {std::type_index(typeid(IntVector2D)), ShaderDataType::IVec2},
                    {std::type_index(typeid(IntVector3D)), ShaderDataType::IVec3},
                    {std::type_index(typeid(IntVector4D)), ShaderDataType::IVec4},
                    {std::type_index(typeid(UIntVector2D)), ShaderDataType::UVec2},
                    {std::type_index(typeid(UIntVector3D)), ShaderDataType::UVec3},
                    {std::type_index(typeid(UIntVector4D)), ShaderDataType::UVec4},
                    {std::type_index(typeid(BoolVector2D)), ShaderDataType::BVec2},
                    {std::type_index(typeid(BoolVector3D)), ShaderDataType::BVec3},
                    {std::type_index(typeid(BoolVector4D)), ShaderDataType::BVec4},
                    {std::type_index(typeid(Matrix2f)), ShaderDataType::Mat2},
                    {std::type_index(typeid(Matrix3f)), ShaderDataType::Mat3},
                    {std::type_index(typeid(Matrix4f)), ShaderDataType::Mat4},
            };

            result_type.type = types.at(std::type_index(typeid(Type)));
            return result_type;
        }
    };


    struct VertexAtribute {
        ShaderDataType type;
        ArrayIndex offset = 0;
    };

    struct VertexBufferInfo {
        std::vector<VertexAtribute> attributes;
        size_t size;
        ArrayIndex binding = 0;
        BufferUsage mode;
    };

    struct ShaderTextureSampler
    {
        uint_t binding = 0;
    };

    enum BufferBitType : uint_t
    {
        ColorBufferBit = 1,
        DepthBufferBit = 2,
        StencilBufferBit = 4,
    };

    using BufferType = size_t;

    enum class FrameBufferType : uint_t
    {
        Framebuffer = 0,
        DrawFramebuffer = 1,
        ReadFramebuffer = 2,
    };

    enum class FrameBufferAttach : uint_t
    {
        ColorAttachment = 0,
        DepthAttachment = 1,
        StencilAttachment = 2,
        DepthStencilAttachment = 3
    };

    enum class TextureCubeMapFace : byte
    {
        Front = 0,
        Back = 1,
        Up = 2,
        Down = 3,
        Left = 4,
        Right = 5
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

    struct ShaderUniformBuffer {
        String name;
        uint_t binding;
        size_t size;
    };

    using FileBuffer = std::vector<byte>;

    struct ShaderParams {
        struct {
            FileBuffer vertex;
            FileBuffer fragment;
            FileBuffer compute;
            FileBuffer geometry;
        } binaries;

        struct {
            std::vector<FileBuffer> vertex;
            std::vector<FileBuffer> fragment;
            std::vector<FileBuffer> compute;
            std::vector<FileBuffer> geometry;
        } text;

        std::vector<ShaderUniformBuffer> uniform_buffers;
        std::vector<ShaderTextureSampler> texture_samplers;

        std::string name;
        VertexBufferInfo vertex_info;
    };

    enum class StencilOption : byte
    {
        Keep,
        Zero,
        Replace,
        Incr,
        IncrWrap,
        Decr,
        DecrWrap,
        Invert,
    };

    enum class SystemType
    {
        LinuxOS,
        WindowsOS,
        AndroidOS
    };

}// namespace Engine
