import "trinex/material.slang";

uniform Sampler2D base_color;
uniform float3 emissive;
uniform float opacity;
uniform float metalic;
uniform float specular;
uniform float roughness;
uniform float ao;

export Material main<Material : IMaterial>(in IMaterialInput input)
{
	Material material;

	material.base_color      = base_color.Sample(input.vertex.uv0).rgb;
	material.emissive        = emissive;
	material.opacity         = opacity;
	material.metalness       = metalic;
	material.specular        = specular;
	material.roughness       = roughness;
	material.ao              = ao;
	material.position_offset = float3(0, 0, 0);

	return material;
}
