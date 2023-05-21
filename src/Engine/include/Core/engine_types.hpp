#pragma once

#include <Core/etl/stl_wrapper.hpp>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <typeindex>

namespace Engine
{
    using byte        = std::uint8_t;
    using signed_byte = std::int8_t;
    using size_t      = std::uint64_t;
    using Flags       = std::uint64_t;

    using Vector1D = glm::vec1;

    using Point1D      = float;
    using Offset1D     = float;
    using Size1D       = float;
    using Scale1D      = float;
    using Translate1D  = float;
    using EulerAngle1D = float;
    using Distance     = float;

    using Point2D      = glm::vec2;
    using Offset2D     = glm::vec2;
    using Size2D       = glm::vec2;
    using Scale2D      = glm::vec2;
    using Translate2D  = glm::vec2;
    using Vector2D     = glm::vec2;
    using EulerAngle2D = glm::vec2;

    using Point3D      = glm::vec3;
    using Offset3D     = glm::vec3;
    using Size3D       = glm::vec3;
    using Scale3D      = glm::vec3;
    using Translate3D  = glm::vec3;
    using Vector3D     = glm::vec3;
    using EulerAngle3D = glm::vec3;
    using Force        = glm::vec3;
    using LightColor   = glm::vec3;

    using Matrix4f = glm::mat4;
    using Matrix3f = glm::mat3;
    using Matrix2f = glm::mat2;

    using Vector4D = glm::vec4;

    using ArrayIndex          = size_t;
    using ArrayOffset         = size_t;
    using PriorityIndex       = size_t;
    using Counter             = size_t;
    using Index               = size_t;
    using MaterialLayoutIndex = size_t;


    using Quaternion = glm::quat;

    using TextureBindIndex   = byte;
    using BindingIndex       = byte;
    using TextureAttachIndex = byte;

    using AssimpObject = const void*;
    using BitMask      = size_t;
    using String       = std::string;
    using MessageList  = List<String>;
    using PixelRGB     = glm::vec<3, byte, glm::defaultp>;
    using PixelRGBA    = glm::vec<4, byte, glm::defaultp>;
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

    using Identifier    = std::uint64_t;
    using MipMapLevel   = byte;
    using LodLevel      = float;
    using LodBias       = float;
    using EnumerateType = std::uint32_t;

    using short_t  = std::int16_t;
    using ushort_t = std::uint16_t;
    using int_t    = std::int32_t;
    using uint_t   = std::uint32_t;
    using int64_t  = std::int64_t;
    using uint64_t = std::uint64_t;


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

    using Buffer     = Vector<byte>;
    using FileBuffer = Buffer;


    enum class SystemType
    {
        LinuxOS,
        WindowsOS,
        AndroidOS
    };

    enum ColorComponent : EnumerateType
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

    using ObjectSet = Set<class Object*>;

#define TRINEX_ENGINE_FLAG 1414678092U
}// namespace Engine
