#pragma once
#include <Core/engine_types.hpp>
#include <concepts>
#include <type_traits>

namespace Engine
{
	class Archive;
	class Object;
	class SingletoneBase;

	template<typename T>
	struct is_valid_type : std::true_type {
	};

	template<typename T>
	inline constexpr bool is_valid_type_v = is_valid_type<T>::value;

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

	template<template<class...> class, typename...>
	struct is_detected : std::false_type {
	};

	template<template<class...> class Op, typename... Args>
	    requires(is_valid_type_v<Op<Args...>>)
	struct is_detected<Op, Args...> : std::true_type {
	};

	template<template<class...> class Op, typename... Args>
	inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

	template<typename T, typename = void>
	struct is_incomplete : std::true_type {
	};

	template<typename T>
	struct is_incomplete<T, std::void_t<decltype(sizeof(T))>> : std::false_type {
	};

	template<typename T>
	inline constexpr bool is_incomplete_v = is_incomplete<T>::value;

	namespace Concepts
	{
		template<typename T>
		concept is_byte = std::is_integral_v<T> && sizeof(T) == sizeof(byte);

		template<typename T>
		concept is_word = std::is_integral_v<T> && sizeof(T) == sizeof(word);

		template<typename T>
		concept is_dword = std::is_integral_v<T> && sizeof(T) == sizeof(dword);

		template<typename T>
		concept is_qword = std::is_integral_v<T> && sizeof(T) == sizeof(qword);

		template<typename T>
		concept is_float = std::is_same_v<T, float>;

		template<typename T>
		concept is_double = std::is_same_v<T, double> || std::is_same_v<T, long double>;

		template<typename T>
		concept struct_with_custom_allocation = requires(T* mem) {
			{ T::static_constructor() } -> std::same_as<T*>;
			{ T::static_destructor(mem) };
		};

		template<typename T, typename... Args>
		concept is_serializable = requires(T* obj, Args&&... args, Engine::Archive& ar) {
			{ obj->serialize(ar, std::forward<Args>(args)...) } -> std::same_as<bool>;
		};

		template<typename T>
		concept is_reflected_struct = requires(T* obj) {
			{ obj->static_reflection() } -> std::same_as<Engine::Refl::Struct*>;
		};

		template<typename T>
		concept is_reflected_class = requires(T* obj) {
			{ obj->static_reflection() } -> std::same_as<Engine::Refl::Class*>;
		};
	}// namespace Concepts
}// namespace Engine
