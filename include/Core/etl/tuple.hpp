#pragma once
#include <tuple>

namespace Engine
{
	template<typename... Args>
	using Tuple = std::tuple<Args...>;
}
