#ifndef MATERIAL_COMMON
#define MATERIAL_COMMON


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

float3x3 calc_tangent_basis_from_world_normal(float3 normal)
{
    float3 tangent = cross(normal, float3(1, 0, 0));
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, bitangent, normal);
}

float3x3 calc_tangent_basis(float3 normal, float3 tangent)
{
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, bitangent, normal);
}

#endif