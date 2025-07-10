#pragma once
#include <Core/engine_types.hpp>
#include <Core/types/color.hpp>

namespace Engine
{
	struct ENGINE_EXPORT WorldEnvironment {
		LinearColor ambient_color = LinearColor(0.05, 0.05, 0.05, 1.f);
	};
}// namespace Engine
