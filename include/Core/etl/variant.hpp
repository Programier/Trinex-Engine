#pragma once
#include <variant>

namespace Trinex
{
	template<typename... Variants>
	using Variant = std::variant<Variants...>;

	namespace etl
	{
		using std::get;
		using std::get_if;
		using std::visit;
	}// namespace etl
}// namespace Trinex
