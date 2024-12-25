#pragma once
#include <array>
#include <string_view>
#include <utility>

namespace Engine
{
	template<auto value>
	class value_info
	{
		template<std::size_t... indices>
		static consteval auto substring_as_array(std::string_view str, std::index_sequence<indices...>)
		{
			return std::array{str[indices]..., '\0'};
		}

		static consteval auto value_name_array()
		{
#if defined(__clang__)
			constexpr auto prefix   = std::string_view{"[value = "};
			constexpr auto suffix   = std::string_view{"]"};
			constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
			constexpr auto prefix   = std::string_view{"with auto value = "};
			constexpr auto suffix   = std::string_view{"]"};
			constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
			constexpr auto prefix   = std::string_view{"value_info<"};
			constexpr auto suffix   = std::string_view{">::value_name_array(void)"};
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

		static constexpr inline auto m_value = value_name_array();

	public:
		static consteval const char* name()
		{
			return m_value.data();
		}
	};
}// namespace Engine
