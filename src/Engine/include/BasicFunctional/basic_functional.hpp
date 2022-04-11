#pragma once
#include <glm/glm.hpp>
#include <iomanip>
#include <ostream>

#define not_implemented (std::runtime_error(std::string("Not implemented method: ") + __PRETTY_FUNCTION__))

namespace Engine
{
    extern const unsigned int processor_count;
    glm::vec3 get_rotation_from_matrix(const glm::mat4& m);
    glm::mat4 quaternion_matrix(const glm::vec3& rotation);
    float scalar_mult(const glm::vec3& first, const glm::vec3& second);
    float angle_between(glm::vec3 first, glm::vec3 second);
}// namespace Engine

#include <utility>

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

static int glm_print_call_nummber = 0;
static std::size_t glm_print_width = 0;
template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value, std::ostream&>::type operator<<(std::ostream& stream,
                                                                                       const Type& value)
{

    glm_print_call_nummber++;
    if (glm_print_call_nummber == 1)
        glm_print_width = 7 + digits_of_number(value);
    int length = value.length();
    bool contain_glm = is_member_of_glm<decltype(value[0])>::value;

    if (!contain_glm)
        stream << "{";
    for (int i = 0; i < length; i++)
    {
        stream << std::fixed << (contain_glm ? std::setw(0) : std::setw(glm_print_width)) << value[i] << std::flush;
        if (!contain_glm)
            stream << (i == length - 1 ? "}\n" : ", ") << std::flush;
    }
    glm_print_call_nummber--;
    return stream;
}
