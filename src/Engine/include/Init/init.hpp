#pragma once

namespace Engine
{
    extern const unsigned int processor_count;
    bool is_inited();
    bool float_equal(const float& a, const float& b, float e = 0.000001);
}// namespace Engine
