#pragma once
#include <tuple>

namespace Trinex
{
	template<typename... Args>
	using Tuple = std::tuple<Args...>;

	namespace etl
	{
		using std::get;
	}
}// namespace Trinex
