module;
#include <array>

export module Trinex.Core.ETL.StaticArray;

export namespace Trinex
{
	template<typename Type, std::size_t size>
	using StaticArray = std::array<Type, size>;

	namespace etl
	{
		using std::to_array;
	}
}
