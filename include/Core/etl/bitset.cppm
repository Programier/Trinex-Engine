module;
#include <Core/etl/std.hpp>
export module Trinex.Core.ETL:Bitset;

export namespace Trinex
{
	template<std::size_t size>
	using BitSet = std::bitset<size>;
}
