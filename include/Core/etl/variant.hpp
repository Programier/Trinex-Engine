#pragma once
#include <variant>

namespace Engine
{
	template<typename... Variants>
	using Variant = std::variant<Variants...>;

	namespace etl
	{
		using std::get;
	}// namespace etl
}// namespace Engine
