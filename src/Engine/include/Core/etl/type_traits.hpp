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
            T, std::conditional_t<false,
                                  is_container_helper<decltype(std::declval<T>().size()), decltype(std::declval<T>().begin()),
                                                      decltype(std::declval<T>().end()), typename T::value_type>,
                                  void>> : public std::true_type {
    };

    template<typename T>
    inline constexpr bool is_container_v = is_container<T>::value;

    template<typename T>
    struct add_pointer_unique {
        using type = T*;
    };

    template<typename T>
    struct add_pointer_unique<T*> {
        using type = T*;
    };

    class Object;
    class SingletoneBase;

    template<typename T>
    using is_object_based = std::is_base_of<Object, T>;

    template<typename T>
    inline constexpr bool is_object_based_v = is_object_based<T>::value;


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

    template<typename T, typename = void>
    struct has_super_type : std::false_type {
    };

    template<typename T>
    struct has_super_type<T, std::void_t<typename T::Super>> : std::true_type {
    };

    template<typename T>
    inline constexpr bool has_super_type_v = has_super_type<T>::value;

    template<typename T>
    inline constexpr bool is_singletone_v = std::is_base_of_v<SingletoneBase, T>;
}// namespace Engine
