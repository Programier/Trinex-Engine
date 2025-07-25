#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct PostProcessParameters final {
		PostProcessParameters& blend(const PostProcessParameters& parameters, float blend_factor);
	};
}// namespace Engine
