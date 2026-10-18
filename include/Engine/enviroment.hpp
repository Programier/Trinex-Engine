#pragma once
#include <Core/engine_types.hpp>
#include <Core/types/color.hpp>

namespace Trinex
{
	struct trinex_api WorldEnvironment {
		LinearColor ambient_color = LinearColor(0.05, 0.05, 0.05, 1.f);
	};
}// namespace Trinex
