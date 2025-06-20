import "trinex/material.slang";

export struct VertexInput : IVertexInput
{
	float2 uv : TEXCOORD0;
}

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

	VertexInput vs_input = (input.vertex_input as VertexInput).value;

	material.base_color      = base_color.Sample(vs_input.uv).rgb;
	material.emissive        = emissive;
	material.opacity         = opacity;
	material.metalness       = metalic;
	material.specular        = specular;
	material.roughness       = roughness;
	material.ao              = ao;
	material.position_offset = float3(0, 0, 0);

	return material;
}
