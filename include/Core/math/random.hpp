#pragma once
#include <Core/engine_types.hpp>

namespace Engine::Random
{
	float_t floating(float min = 0.f, float max = 1.f);
	int_t integer(int_t min = 0, int_t max = 2147483647);
}// namespace Engine::Random
