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
    using EulerAngle2D = glm::vec2;

    using Point3D      = glm::vec3;
    using Offset3D     = glm::vec3;
    using Size3D       = glm::vec3;
    using Scale3D      = glm::vec3;
    using Translate3D  = glm::vec3;
    using EulerAngle3D = glm::vec3;
    using Force        = glm::vec3;
    using LightColor   = glm::vec3;

    using Matrix4f = glm::mat4;
    using Matrix3f = glm::mat3;
    using Matrix2f = glm::mat2;

    using Matrix4x2f = glm::mat4x2;
    using Matrix4x3f = glm::mat4x3;
    using Matrix4x4f = glm::mat4x4;

    using Matrix3x2f = glm::mat3x2;
    using Matrix3x3f = glm::mat3x3;
    using Matrix3x4f = glm::mat3x4;

    using Matrix2x2f = glm::mat2x2;
    using Matrix2x3f = glm::mat2x3;
    using Matrix2x4f = glm::mat2x4;


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


    using Vector1D = glm::vec1;
    using Vector2D = glm::vec2;
    using Vector3D = glm::vec3;
    using Vector4D = glm::vec4;

    using BoolVector1D = glm::bvec1;
    using BoolVector2D = glm::bvec2;
    using BoolVector3D = glm::bvec3;
    using BoolVector4D = glm::bvec4;

    // Int Vectors
    using IntVector1D = glm::ivec1;
    using IntVector2D = glm::ivec2;
    using IntVector3D = glm::ivec3;
    using IntVector4D = glm::ivec4;

    using UIntVector1D = glm::uvec1;
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

        SizeLimits(Type&& _min, Type&& _max) : min(std::move(_min)), max(std::move(_max))
        {}

        SizeLimits(SizeLimits&&)      = default;
        SizeLimits(const SizeLimits&) = default;


        SizeLimits& operator=(const SizeLimits&) = default;
        SizeLimits& operator=(SizeLimits&&)      = default;
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


// PRINTING GLM OBJECT
template<typename T, typename = void>
struct is_member_of_glm : std::false_type {
};

template<typename T>
struct is_member_of_glm<T, decltype(adl_member_of_glm(std::declval<T>()))> : std::true_type {
};


namespace glm
{
    template<typename T>
    auto adl_member_of_glm(T&&) -> void;
}

template<typename Number>
typename std::enable_if<std::is_arithmetic<Number>::value, int>::type digits_of_number(const Number& number)
{
    signed long int value = static_cast<signed long int>(number);
    int digits            = value <= 0 ? 1 : 0;
    while (value != 0)
    {
        digits++;
        value /= 10;
    }
    return digits;
}

template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value, int>::type digits_of_number(const Type& value)
{
    int length = value.length();
    int digits = 0;
    for (int i = 0; i < length; i++) digits = std::max(digits, digits_of_number(value[i]));
    return digits;
}

// Printing glm value

template<typename Type>
typename std::enable_if<!is_member_of_glm<Type>::value, std::ostream&>::type
print_glm_object(std::ostream& stream, const Type& value, const std::size_t& glm_print_width = 0)
{
    return stream << std::fixed << std::setw(glm_print_width) << value << std::flush;
}

template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
print_glm_object(std::ostream& stream, const Type& value, std::size_t glm_print_width = 0)

{

    if (glm_print_width == 1)
        glm_print_width = 7 + digits_of_number(value);
    int length       = value.length();
    bool contain_glm = is_member_of_glm<decltype(value[0])>::value;

    if (!contain_glm)
        stream << "{";
    for (int i = 0; i < length; i++)
    {
        print_glm_object(stream, value[i], glm_print_width);

        if (!contain_glm)
            stream << (i == length - 1 ? "}" : ", ") << std::flush;
        else
            stream << std::endl;
    }
    return stream;
}

namespace glm
{
    template<typename Type>
    typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
    operator<<(std::ostream& stream, const Type& value)
    {
        return print_glm_object(stream, value);
    }
}// namespace glm
