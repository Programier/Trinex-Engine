#ifndef PLATFORM_SLANG
#define PLATFORM_SLANG 0

float2 platform_validate_uv(in float2 uv)
{
#ifdef TRINEX_INVERT_UV
    return float2(uv.x, 1.0 - uv.y);
#else
    return uv;
#endif
}


#endif