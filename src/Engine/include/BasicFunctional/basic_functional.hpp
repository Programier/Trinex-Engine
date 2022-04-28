#pragma once
#include <glm/glm.hpp>
#include <iomanip>
#include <ostream>
#include <utility>

#define not_implemented                                                                                                               \
    (std::runtime_error(std::string(__FILE__) + str::string(":") + std::string(__LINE__) + std::string(" Not implemented method: ") + \
                        __PRETTY_FUNCTION__))

namespace Engine
{
    enum class Coord
    {
        X,
        Y,
        Z
    };

    extern const unsigned int processor_count;
    extern const glm::vec3 OX;
    extern const glm::vec3 OY;
    extern const glm::vec3 OZ;
    extern const float PI;
    extern const float E;
    extern const glm::mat4 identity_matrix;
    extern const glm::mat4 zero_matrix;
    extern const glm::vec4 identity_vector;
    extern const glm::vec4 zero_vector;


    glm::vec3 get_rotation_from_matrix(const glm::mat4& m);
    glm::mat4 quaternion_matrix(const glm::vec3& rotation);
    float scalar_mult(const glm::vec3& first, const glm::vec3& second);
    float angle_between(glm::vec3 first, glm::vec3 second);
    glm::vec3 remove_coord(const glm::vec3& vector, const Coord& coord);
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
    int digits = value <= 0 ? 1 : 0;
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
typename std::enable_if<!is_member_of_glm<Type>::value, std::ostream&>::type print_glm_object(std::ostream& stream, const Type& value,
                                                                                              const std::size_t& glm_print_width = 0)
{
    return stream << std::fixed << std::setw(glm_print_width) << value << std::flush;
}

template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
print_glm_object(std::ostream& stream, const Type& value, std::size_t glm_print_width = 0)

{

    if (glm_print_width == 1)
        glm_print_width = 7 + digits_of_number(value);
    int length = value.length();
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

template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
operator<<(std::ostream& stream, const Type& value)
{
    return print_glm_object(stream, value);
}

namespace Engine
{
    template<typename Type>
    typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
    operator<<(std::ostream& stream, const Type& value)
    {
        return print_glm_object(stream, value);
    }
}// namespace Engine
