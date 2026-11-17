module;
#include <Core/etl/std.hpp>
export module Trinex.Core.ETL:Utility;

export namespace Trinex
{
	template<typename T1, typename T2>
	using Pair = std::pair<T1, T2>;

	namespace etl
	{
		using std::forward;
		using std::index_sequence;
		using std::index_sequence_for;
		using std::make_index_sequence;
		using std::move;
		using std::swap;
		using std::declval;
	} // namespace etl
} // namespace Trinex
