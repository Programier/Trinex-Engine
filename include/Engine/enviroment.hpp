#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct ENGINE_EXPORT WorldEnvironment {
		LinearColor ambient_color = LinearColor(0.3, 0.3, 0.3, 1.f);
	};
}// namespace Engine
