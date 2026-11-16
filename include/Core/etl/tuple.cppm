module;
#include <tuple>

export module Trinex.Core.ETL.Tuple;

export namespace Trinex
{
	template<typename... Args>
	using Tuple = std::tuple<Args...>;

	namespace etl
	{
		using std::get;
	}
} // namespace Trinex
