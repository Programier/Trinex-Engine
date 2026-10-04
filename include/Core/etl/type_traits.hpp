#pragma once
#include <type_traits>

namespace Trinex::etl
{
	using std::bool_constant;
	using std::common_type_t;
	using std::conditional_t;
	using std::decay_t;
	using std::extent_v;
	using std::false_type;
	using std::integral_constant;
	using std::invoke_result_t;
	using std::is_aggregate_v;
	using std::is_arithmetic_v;
	using std::is_array_v;
	using std::is_base_of_v;
	using std::is_class_v;
	using std::is_const_v;
	using std::is_constructible_v;
	using std::is_convertible_v;
	using std::is_copy_assignable_v;
	using std::is_copy_constructible_v;
	using std::is_default_constructible_v;
	using std::is_empty_v;
	using std::is_enum_v;
	using std::is_final_v;
	using std::is_function_v;
	using std::is_integral_v;
	using std::is_invocable;
	using std::is_invocable_r;
	using std::is_invocable_r_v;
	using std::is_invocable_v;
	using std::is_lvalue_reference_v;
	using std::is_member_function_pointer_v;
	using std::is_member_object_pointer_v;
	using std::is_member_pointer_v;
	using std::is_move_assignable_v;
	using std::is_move_constructible_v;
	using std::is_nothrow_constructible_v;
	using std::is_nothrow_copy_constructible_v;
	using std::is_nothrow_default_constructible_v;
	using std::is_nothrow_destructible_v;
	using std::is_nothrow_invocable_v;
	using std::is_nothrow_move_constructible_v;
	using std::is_pointer_v;
	using std::is_reference_v;
	using std::is_same_v;
	using std::is_signed_v;
	using std::is_trivially_destructible_v;
	using std::is_void_v;
	using std::is_volatile;
	using std::is_volatile_v;
	using std::remove_const_t;
	using std::remove_cvref_t;
	using std::remove_pointer_t;
	using std::remove_reference_t;
	using std::true_type;
	using std::type_identity;
	using std::underlying_type_t;

	class Archive;
	class Object;

	// ============================================================================
	// Basic utilities
	// ============================================================================

	template<typename...>
	inline constexpr bool always_false_v = false;


	// ============================================================================
	// Object traits
	// ============================================================================

	template<typename T>
	struct is_object_type : std::is_base_of<Object, std::remove_cvref_t<T>> {
	};

	template<typename T>
	inline constexpr bool is_object_type_v = is_object_type<T>::value;


	template<typename T>
	concept object_type = is_object_type_v<T>;


	// ============================================================================
	// Super
	// ============================================================================

	template<typename T, typename = void>
	struct has_super : std::false_type {
	};

	template<typename T>
	struct has_super<T, std::void_t<typename std::remove_cvref_t<T>::Super>> : std::true_type {
	};

	template<typename T>
	inline constexpr bool has_super_v = has_super<T>::value;


	template<typename T>
	using super_t = typename std::remove_cvref_t<T>::Super;


	template<typename T>
	concept object_with_super = object_type<T> && has_super_v<T>;


	// ============================================================================
	// Detection
	// ============================================================================

	template<template<typename...> typename Op, typename... Args>
	concept detected = requires { typename Op<Args...>; };


	template<template<typename...> typename Op, typename... Args>
	struct is_detected : std::bool_constant<detected<Op, Args...>> {
	};

	template<template<typename...> typename Op, typename... Args>
	inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;


	template<typename T, typename... Ts>
	struct is_any_of : std::disjunction<std::is_same<T, Ts>...> {
	};

	template<typename T, typename... Ts>
	inline constexpr bool is_any_of_v = is_any_of<T, Ts...>::value;


	template<typename T, template<typename...> typename Template>
	struct is_specialization_of_impl : std::false_type {
	};

	template<template<typename...> typename Template, typename... Args>
	struct is_specialization_of_impl<Template<Args...>, Template> : std::true_type {
	};


	template<typename T, template<typename...> typename Template>
	struct is_specialization_of : is_specialization_of_impl<std::remove_cvref_t<T>, Template> {
	};

	template<typename T, template<typename...> typename Template>
	inline constexpr bool is_specialization_of_v = is_specialization_of<T, Template>::value;
}// namespace Trinex::etl
