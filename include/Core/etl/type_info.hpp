#pragma once
#include <Core/export.hpp>
#include <array>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Engine
{
	class Object;

	struct ENGINE_EXPORT type_info_base {
	protected:
		template<std::size_t... indices>
		static consteval auto substring_as_array(std::string_view str, std::index_sequence<indices...>)
		{
			return std::array{str[indices]..., '\0'};
		}

		template<typename T>
		static consteval auto type_name_array()
		{
#if defined(__clang__)
			constexpr auto prefix   = std::string_view{"[T = "};
			constexpr auto suffix   = std::string_view{"]"};
			constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
			constexpr auto prefix   = std::string_view{"with T = "};
			constexpr auto suffix   = std::string_view{"]"};
			constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
			constexpr auto prefix   = std::string_view{"type_name_array<"};
			constexpr auto suffix   = std::string_view{">(void)"};
			constexpr auto function = std::string_view{__FUNCSIG__};
#else
#error Unsupported compiler
#endif

			constexpr auto start = function.find(prefix) + prefix.size();
			constexpr auto end   = function.rfind(suffix);
			static_assert(start < end);

			constexpr auto name = function.substr(start, end - start);
			return substring_as_array(name, std::make_index_sequence<name.size()>{});
		}
	};

	template<typename T>
	class type_info : public type_info_base
	{
		static constexpr inline auto m_name = type_name_array<std::decay_t<T>>();

	public:
		using value_type = T;
		using decay_type = std::decay_t<T>;

		static constexpr inline bool is_void                            = std::is_void_v<T>;
		static constexpr inline bool is_bool                            = std::is_same_v<decay_type, bool>;
		static constexpr inline bool is_integral                        = std::is_integral_v<T>;
		static constexpr inline bool is_floating_point                  = std::is_floating_point_v<T>;
		static constexpr inline bool is_array                           = std::is_array_v<T>;
		static constexpr inline bool is_pointer                         = std::is_pointer_v<T>;
		static constexpr inline bool is_reference                       = std::is_reference_v<T>;
		static constexpr inline bool is_const                           = std::is_const_v<T>;
		static constexpr inline bool is_volatile                        = std::is_volatile_v<T>;
		static constexpr inline bool is_signed                          = std::is_signed_v<T>;
		static constexpr inline bool is_unsigned                        = std::is_unsigned_v<T>;
		static constexpr inline bool is_class                           = std::is_class_v<T>;
		static constexpr inline bool is_union                           = std::is_union_v<T>;
		static constexpr inline bool is_enum                            = std::is_enum_v<T>;
		static constexpr inline bool is_function                        = std::is_function_v<T>;
		static constexpr inline bool is_member_pointer                  = std::is_member_pointer_v<T>;
		static constexpr inline bool is_member_function_pointer         = std::is_member_function_pointer_v<T>;
		static constexpr inline bool is_fundamental                     = std::is_fundamental_v<T>;
		static constexpr inline bool is_arithmetic                      = std::is_arithmetic_v<T>;
		static constexpr inline bool is_scalar                          = std::is_scalar_v<T>;
		static constexpr inline bool is_object                          = std::is_object_v<T>;
		static constexpr inline bool is_trivially_copyable              = std::is_trivially_copyable_v<T>;
		static constexpr inline bool is_standard_layout                 = std::is_standard_layout_v<T>;
		static constexpr inline bool is_empty                           = std::is_empty_v<T>;
		static constexpr inline bool is_polymorphic                     = std::is_polymorphic_v<T>;
		static constexpr inline bool is_abstract                        = std::is_abstract_v<T>;
		static constexpr inline bool is_final                           = std::is_final_v<T>;
		static constexpr inline bool is_aggregate                       = std::is_aggregate_v<T>;
		static constexpr inline bool is_default_constructible           = std::is_default_constructible_v<T>;
		static constexpr inline bool is_trivially_default_constructible = std::is_trivially_default_constructible_v<T>;
		static constexpr inline bool is_nothrow_default_constructible   = std::is_nothrow_default_constructible_v<T>;
		static constexpr inline bool is_copy_constructible              = std::is_copy_constructible_v<T>;
		static constexpr inline bool is_trivially_copy_constructible    = std::is_trivially_copy_constructible_v<T>;
		static constexpr inline bool is_nothrow_copy_constructible      = std::is_nothrow_copy_constructible_v<T>;
		static constexpr inline bool is_move_constructible              = std::is_move_constructible_v<T>;
		static constexpr inline bool is_trivially_move_constructible    = std::is_trivially_move_constructible_v<T>;
		static constexpr inline bool is_nothrow_move_constructible      = std::is_nothrow_move_constructible_v<T>;
		static constexpr inline bool is_copy_assignable                 = std::is_copy_assignable_v<T>;
		static constexpr inline bool is_trivially_copy_assignable       = std::is_trivially_copy_assignable_v<T>;
		static constexpr inline bool is_nothrow_copy_assignable         = std::is_nothrow_copy_assignable_v<T>;
		static constexpr inline bool is_move_assignable                 = std::is_move_assignable_v<T>;
		static constexpr inline bool is_trivially_move_assignable       = std::is_trivially_move_assignable_v<T>;
		static constexpr inline bool is_nothrow_move_assignable         = std::is_nothrow_move_assignable_v<T>;
		static constexpr inline bool is_destructible                    = std::is_destructible_v<T>;
		static constexpr inline bool is_trivially_destructible          = std::is_trivially_destructible_v<T>;
		static constexpr inline bool is_nothrow_destructible            = std::is_nothrow_destructible_v<T>;
		static constexpr inline bool is_object_based                    = std::is_base_of_v<Object, decay_type>;

		static consteval std::string_view name()
		{
			return m_name.data();
		}

		consteval bool operator==(const type_info& info) const
		{
			return name() == info.name();
		}

		consteval bool operator!=(const type_info& info) const
		{
			return name() != info.name();
		}
	};
}// namespace Engine
