#pragma once
#include <functional>

namespace Engine
{
	template<typename Signature>
	using Function = std::function<Signature>;
}
