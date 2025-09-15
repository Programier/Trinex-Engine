#pragma once
#include <variant>

namespace Engine
{
	template<typename... Variants>
	using Variant = std::variant<Variants...>;

	namespace etl
	{
		template<typename T, typename... Args>
		constexpr inline T& get(Variant<Args...>& v)
		{
			return std::get<T>(v);
		}

		template<typename T, typename... Args>
		constexpr inline const T& get(const Variant<Args...>& v)
		{
			return std::get<T>(v);
		}
	}// namespace etl
}// namespace Engine
