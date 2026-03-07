#pragma once
#include <array>

namespace Trinex
{
	template<typename Type, std::size_t size>
	using Array = std::array<Type, size>;
}
