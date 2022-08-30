#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Engine
{
    typedef unsigned char byte;

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
    typedef glm::vec<4, float, glm::defaultp> Color;


    typedef std::size_t ArrayIndex;

    typedef glm::quat Quaternion;

    typedef unsigned char TextureBindIndex;

    typedef struct {
        float min;
        float max;
    } SizeLimits1D;

    typedef struct {
        Point2D min;
        Point2D max;
    } SizeLimits2D;

    typedef struct {
        Point3D min;
        Point3D max;
    } SizeLimits3D;


    typedef SizeLimits1D AABB_1D;
    typedef SizeLimits2D AABB_2D;
    typedef SizeLimits3D AABB_3D;

    typedef unsigned int ObjectID;
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


    struct OpenGL_Version_S
    {
        int major;
        int minor;
    };
}// namespace Engine
