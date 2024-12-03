#pragma once
#include <bitset>

namespace Engine
{
	template<std::size_t size>
	using BitSet = std::bitset<size>;
}
