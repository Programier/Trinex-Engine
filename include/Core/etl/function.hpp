#pragma once
#include <functional>

namespace Trinex
{
	template<typename Signature>
	using Function = std::function<Signature>;
}
