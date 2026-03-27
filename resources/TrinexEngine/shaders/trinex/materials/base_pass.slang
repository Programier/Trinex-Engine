import "trinex/material.slang";

struct BaseColor
{
	Sampler2D<float3> texture;
	float3 factor;

	[ForceInline] float3 sample(float2 uv) { return texture.Sample(uv) * factor; }
};

struct MetalicRoughness
{
	Sampler2D texture;
	float2 factor;

	[ForceInline] float2 sample(float2 uv) { return texture.Sample(uv).bg * factor; }
};

uniform BaseColor base_color;
uniform MetalicRoughness metalic_roughness;
uniform Sampler2D<float3> normal;

export Material main<Material : IMaterial>(in IMaterialInput input)
{
	Material material;

	const float2 uv = input.vertex.uv0;

	float2 mr = metalic_roughness.sample(uv);

	material.base_color      = base_color.sample(uv);
	material.normal          = normal.Sample(uv);
	material.emissive        = float3(0.f, 0.f, 0.f);
	material.opacity         = 1.f;
	material.metalness       = mr.x;
	material.specular        = 0.5;
	material.roughness       = mr.y;
	material.ao              = 1.f;
	material.position_offset = float3(0, 0, 0);
	
	return material;
}


