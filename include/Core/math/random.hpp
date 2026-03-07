#pragma once
#include <Core/engine_types.hpp>

namespace Trinex::Random
{
	f32 floating(float min = 0.f, float max = 1.f);
	i32 integer(i32 min = 0, i32 max = 2147483647);
}// namespace Trinex::Random
