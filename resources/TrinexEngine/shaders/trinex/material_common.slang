#ifndef MATERIAL_COMMON
#define MATERIAL_COMMON
#include "trinex/definitions.slang"

struct Material
{
	float3 base_color;
	float3 emissive;
	float3 normal;
	float3 position_offset;

	float specular;
	float metalness;
	float roughness;
	float opacity;
    float AO;
};

float3x3 calc_tangent_basis(float3 normal, float3 pos, float2 uv)
{
    float3 dp1 = ddx(pos);
    float3 dp2 = ddy(pos);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);
    float3 dp2perp = cross(dp2, normal);
    float3 dp1perp = cross(normal, dp1);
    float3 tangent = normalize(dp2perp * duv1.x + dp1perp * duv2.x);
    float3 bitangent = normalize(dp2perp * duv1.y + dp1perp * duv2.y);
    return float3x3(tangent, bitangent, normal);
}

float3x3 calc_tangent_basis(float3 normal, float3 tangent, float3 bitangent)
{
    float3x3 result = float3x3(tangent, bitangent, normal);
#if TRINEX_VULKAN_RHI || TRINEX_OPENGL_RHI
    return transpose(result);
#else
    return result;
#endif
}

float3x3 calc_tangent_basis(float3 normal, float3 tangent)
{
    return calc_tangent_basis(normal, tangent, cross(normal, tangent));
}

float3x3 calc_tangent_basis(float3 normal)
{
    return calc_tangent_basis(normal, cross(normal, float3(1, 0, 0)));
}
#endif
