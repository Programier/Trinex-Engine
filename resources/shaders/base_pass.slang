#define DISABLE_TANGENT_AND_BINORMAL 1

#include "common.slang"
#include VERTEX_FACTORY_FILE

struct VertexOutput
{
	float4 position 		: SV_Position;
	float4 world_position 	: POSITION0;
	float3 world_normal 	: NORMAL0;
	float2 uv 				: TEXCOORD0;
};

uniform Sampler2D base_color;
uniform Sampler2D emissive;
uniform Sampler2D opacity;
uniform Sampler2D metalic;
uniform Sampler2D specular;
uniform Sampler2D roughness;
uniform Sampler2D AO;


[shader("vertex")]
VertexOutput vs_main(in VertexFactory input)
{
	VertexOutput output;

	float4 world_position = input.get_world_position();
	output.world_position = world_position;
	output.position = mul(globals.projview, world_position);
	output.world_normal = input.get_world_normal();
	output.uv =		input.get_uv(0);

	return output;
}

[shader("fragment")]
GBufferFragmentOutput fs_main(in VertexOutput input, in bool IsFrontFace : SV_IsFrontFace)
{
	GBufferFragmentOutput output;
	float2 uv = input.uv;

	output.base_color = float4(base_color.Sample(uv).rgb, opacity.Sample(uv).r);
    output.position = input.world_position;
    output.normal = float4(input.world_normal, IsFrontFace ? 1.0 : -1.0);
    output.emissive = emissive.Sample(uv);
    output.msra = float4(metalic.Sample(uv).r, specular.Sample(uv).r, roughness.Sample(uv).r, AO.Sample(uv).r);
	return output;
}