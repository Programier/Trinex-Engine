#pragma once
#include <array>

namespace Engine
{
	template<typename Type, std::size_t size>
	using Array = std::array<Type, size>;
}
