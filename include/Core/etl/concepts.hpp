#pragma once

#pragma once

#include <concepts>

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

}// namespace Trinex::etl
