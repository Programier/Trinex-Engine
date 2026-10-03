#pragma once

#include <concepts>

namespace Trinex
{
	class Archive;
}

namespace Trinex::etl
{
	using std::same_as;

	using std::derived_from;

	using std::convertible_to;

	using std::common_reference_with;
	using std::common_with;

	using std::floating_point;
	using std::integral;
	using std::signed_integral;
	using std::unsigned_integral;

	using std::assignable_from;

	using std::swappable;
	using std::swappable_with;

	using std::destructible;

	using std::constructible_from;
	using std::default_initializable;

	using std::copy_constructible;
	using std::move_constructible;

	using std::equality_comparable;
	using std::equality_comparable_with;

	using std::totally_ordered;
	using std::totally_ordered_with;

	using std::copyable;
	using std::movable;

	using std::regular;
	using std::semiregular;

	using std::invocable;
	using std::regular_invocable;

	using std::predicate;

	using std::equivalence_relation;
	using std::relation;
	using std::strict_weak_order;

	template<typename T>
	concept arithmetic = integral<T> || floating_point<T>;

	template<typename T>
	concept reflected_enum = requires {
		typename std::remove_cvref_t<T>::Enum;
		requires std::is_enum_v<typename std::remove_cvref_t<T>::Enum>;
		requires std::remove_cvref_t<T>::is_enum;
		requires std::remove_cvref_t<T>::is_enum_reflected;
	};

	template<typename T>
	concept regular_reflected_enum = reflected_enum<T> && !std::remove_cvref_t<T>::is_bitfield_enum;

	template<typename T>
	concept bitfield_reflected_enum = reflected_enum<T> && std::remove_cvref_t<T>::is_bitfield_enum;

	template<typename T>
	concept serializable_member = requires(T& value, class Archive& archive) { value.serialize(archive); };

	template<typename T>
	concept serializable_free = requires(T& value, class Archive& archive) { serialize(archive, value); };

	template<typename T>
	concept serializable_primitive =
	        std::is_trivially_copyable_v<std::remove_cvref_t<T>> && !std::is_pointer_v<std::remove_cvref_t<T>> &&
	        !std::is_member_pointer_v<std::remove_cvref_t<T>>;

	template<typename T>
	concept is_byte = std::is_integral_v<T> && sizeof(T) == sizeof(u8);

	template<typename T>
	concept is_word = std::is_integral_v<T> && sizeof(T) == sizeof(u16);

	template<typename T>
	concept is_dword = std::is_integral_v<T> && sizeof(T) == sizeof(u32);

	template<typename T>
	concept is_qword = std::is_integral_v<T> && sizeof(T) == sizeof(u64);

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
	concept is_serializable = requires(T* obj, Args&&... args, Trinex::Archive& ar) {
		{ obj->serialize(ar, std::declval<Args>(args)...) } -> std::same_as<bool>;
	};

	template<typename T>
	concept is_reflected_struct = requires(T* obj) {
		{ obj->static_reflection() } -> std::same_as<Trinex::Refl::Struct*>;
	};

	template<typename T>
	concept is_reflected_class = requires(T* obj) {
		{ obj->static_reflection() } -> std::same_as<Trinex::Refl::Class*>;
	};
}// namespace Trinex::etl
