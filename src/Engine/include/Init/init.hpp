#pragma once
#include <glm/glm.hpp>

namespace Engine
{
    bool is_inited();
    bool float_equal(const float& a, const float& b, float e = 0.000001);
}// namespace Engine
