#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>


namespace Engine
{
    typedef std::uint8_t byte;

    typedef float Point1D;
    typedef float Offset1D;
    typedef float Size1D;
    typedef float Scale1D;
    typedef float Translate1D;
    typedef float Vector1D;
    typedef float EulerAngle1D;
    typedef float Distance;

    typedef glm::vec2 Point2D;
    typedef glm::vec2 Offset2D;
    typedef glm::vec2 Size2D;
    typedef glm::vec2 Scale2D;
    typedef glm::vec2 Translate2D;
    typedef glm::vec2 Vector2D;
    typedef glm::vec2 EulerAngle2D;

    typedef glm::vec3 Point3D;
    typedef glm::vec3 Offset3D;
    typedef glm::vec3 Size3D;
    typedef glm::vec3 Scale3D;
    typedef glm::vec3 Translate3D;
    typedef glm::vec3 Vector3D;
    typedef glm::vec3 EulerAngle3D;
    typedef glm::vec3 Force;
    typedef glm::vec3 LightColor;


    typedef glm::vec4 Vector4D;

    typedef std::size_t ArrayIndex;

    typedef glm::quat Quaternion;

    typedef unsigned char TextureBindIndex;

    template<typename Type>
    class SizeLimits
    {
    public:
        Type min;
        Type max;

        SizeLimits() = default;
        SizeLimits(const Type& _min, const Type& _max) : min(_min), max(_max) {}

        Type& operator[](bool _max)
        {
            if(_max)
                return max;
            return min;
        }

        const Type& operator[](bool _max) const
        {
            if(_max)
                return max;
            return min;
        }
    };

    using SizeLimits1D = SizeLimits<Size1D>;
    using SizeLimits2D = SizeLimits<Size2D>;
    using SizeLimits3D = SizeLimits<Size3D>;


    typedef SizeLimits1D AABB_1D;
    typedef SizeLimits2D AABB_2D;
    typedef SizeLimits3D AABB_3D;

    typedef uint32_t ObjectID;
    typedef std::uintptr_t ObjID;
    typedef unsigned int MipMapLevel;

    enum class Coord
    {
        X,
        Y,
        Z
    };

    typedef std::size_t BufferType;

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

    enum class EngineAPI : unsigned int
    {
        OpenGL = 0,
        Vulkan = 1
    };

    typedef glm::vec<2, int, glm::defaultp> AspectRation;

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
        int major;
        int minor;
    };

    enum WindowOrientation : unsigned int
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

    enum class PixelFormat : unsigned int
    {
        DEPTH = 1,
        RGB = 3,
        RGBA = 4,
        RED = 5,
        GREEN = 6,
        BLUE = 7,
        ALPHA = 8
    };

    enum class BufferValueType : unsigned int
    {
        FLOAT = 0,
        UNSIGNED_BYTE = 1,
        UNSIGNED_SHORT = 2,
        UNSIGNED_INT = 3,
        SHORT = 4,
        INT = 5
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

    enum class TextureFilter : unsigned int
    {
        NEAREST = 0,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_NEAREST,
        LINEAR_MIPMAP_LINEAR
    };


    struct SwizzleRGBA {
        enum class SwizzleValue : unsigned int
        {
            RED = 0,
            GREEN = 1,
            BLUE = 2,
            ALPHA = 3
        } R, G, B, A;
    };

    enum class WrapValue : unsigned int
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

    enum class DrawMode
    {
        STATIC_DRAW,
        DYNAMIC_DRAW
    };


    struct MeshAtribute {
        int count;
        BufferValueType type;
    };

    struct MeshInfo {
        std::size_t vertices = 0;
        std::vector<MeshAtribute> attributes;
        DrawMode mode;
    };

    enum BufferBitType : unsigned int
    {
        COLOR_BUFFER_BIT = 1,
        DEPTH_BUFFER_BIT = 2,
    };

    enum class FrameBufferType : unsigned int
    {
        FRAMEBUFFER = 0,
        DRAW_FRAMEBUFFER = 1,
        READ_FRAMEBUFFER = 2,
    };

    enum class FrameBufferAttach : unsigned int
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

    enum class EnableCap : unsigned int
    {
        Blend = 0,
        DepthTest = 1,
        CullFace = 2
    };

    enum class BlendFunc : unsigned int
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


    enum class DataType{
        Text = 0,
        Binary
    };


    using ShaderSourceType = DataType;
    using FileBuffer = std::vector<byte>;

    struct ShaderParams
    {
        ShaderSourceType source_type;
        std::string name;
        FileBuffer vertex;
        FileBuffer fragment;
        FileBuffer compute;
        FileBuffer geometry;
    };
}// namespace Engine
