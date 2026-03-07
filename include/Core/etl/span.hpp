#pragma once
#include <span>

namespace Trinex
{
	template<typename Type, std::size_t extend = std::dynamic_extent>
	using Span = std::span<Type, extend>;
}
