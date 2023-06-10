#pragma once
#include <type_traits>

namespace Engine
{
    template<typename T, typename _ = void>
    struct is_container : std::false_type {
    };

    template<typename... Ts>
    struct is_container_helper {
    };

    template<typename T>
    struct is_container<
            T, std::conditional_t<
                       false,
                       is_container_helper<decltype(std::declval<T>().size()), decltype(std::declval<T>().begin()),
                                           decltype(std::declval<T>().end()), typename T::value_type>,
                       void>> : public std::true_type {
    };

    template<typename T>
    struct add_pointer_unique {
        using type = T*;
    };

    template<typename T>
    struct add_pointer_unique<T*> {
        using type = T*;
    };

    template<typename T, typename _ = void>
    struct is_object_based : std::false_type {
    };


    template<typename T>
    struct is_object_based<T, std::conditional_t<false, typename T::ObjectClass, void>> : public std::true_type {
    };

    template<typename Type>
    struct is_function_reference : std::false_type {
    };

    template<typename Type, typename... Args>
    struct is_function_reference<Type (&)(Args...)> : std::true_type {
    };

    template<typename Type>
    constexpr bool is_function_reference_v = is_function_reference<Type>::value;

    template<typename T>
    struct is_string_literal : std::false_type {
    };

    template<std::size_t N>
    struct is_string_literal<const char (&)[N]> : std::true_type {
    };

    template<std::size_t N>
    struct is_string_literal<const wchar_t (&)[N]> : std::true_type {
    };

    template<typename T>
    constexpr bool is_string_literal_v = is_string_literal<T>::value;
}// namespace Engine
