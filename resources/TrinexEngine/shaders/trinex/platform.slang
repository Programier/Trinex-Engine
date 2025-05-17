#ifndef PLATFORM_SLANG
#define PLATFORM_SLANG 1

#include "trinex/definitions.slang"

namespace Platform
{
#if TRINEX_OPENGL_RHI
    static const float2 ndc_depth_range = float2(-1.f, 1.f);
#else
    static const float2 ndc_depth_range = float2(0.f, 1.f);
#endif

    float2 validate_uv(in float2 uv)
    {
#if TRINEX_INVERT_UV
        return float2(uv.x, 1.0 - uv.y);
#else
        return uv;
#endif
    }

    [ForceInline] float linearize_depth(float depth, float near, float far)
    {
#if TRINEX_OPENGL_RHI
        float z = depth * 2.0 - 1.0;
        return (2.0 * near * far) / (far + near - z * (far - near));
#else
        return (near * far) / (far + depth * (near - far));
#endif
    }
}

#endif
