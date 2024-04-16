#include "common.slang"

struct VertexInput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
};

struct VertexOutput
{
	float4 position : SV_Position;
	float3 world_position;
	float3 normal : NORMAL0;
};

uniform float4x4 model;

uniform float4 emissive;
uniform float3 color;
uniform float opacity;
uniform float metalic;
uniform float specular;
uniform float roughness;
uniform float AO;


[shader("vertex")]
VertexOutput vs_main(in VertexInput input)
{
	VertexOutput output;

	float4 tmp_position = mul(model, float4(input.position, 1.0));
	output.world_position = tmp_position.xyz;
	output.position = mul(globals.projview, tmp_position);
	output.normal = rotate_normal(input.normal, model);

	return output;
}

[shader("fragment")]
GBufferFragmentOutput fs_main(in VertexOutput input, in bool IsFrontFace : SV_IsFrontFace)
{
	GBufferFragmentOutput output;
	output.base_color = float4(color, opacity);
    output.position = float4(input.world_position, 1.0);
    output.normal = float4(input.normal, IsFrontFace ? 1.0 : 1.0);
    output.emissive = emissive;
    output.msra = float4(metalic, specular, roughness, AO);
	return output;
}