#define DISABLE_TANGENT_AND_BITANGENT 1

#include "trinex/attributes.slang"
#include "common.slang"
#include "platform.slang"

#include VERTEX_FACTORY_FILE

struct VertexOutput
{
	float4 position 		: SV_Position;
	float4 world_position 	: POSITION0;
	float3 world_normal 	: NORMAL0;
	float2 uv 				: TEXCOORD0;
};

[is_globals()]
ConstantBuffer<GlobalParameters> globals;

uniform Sampler2D base_color;
uniform float3 emissive;
uniform float opacity;
uniform float metalic;
uniform float specular;
uniform float roughness;
uniform float ao;

[shader("vertex")]
VertexOutput vs_main(in VertexFactory input)
{
	VertexOutput output;

	float4 world_position = float4(input.get_world_position(), 1.0);
	output.world_position = world_position;
	output.position = mul(globals.projview, world_position);
	output.world_normal = input.get_world_normal();
	output.uv =	Platform::validate_uv(input.get_uv(0));

	return output;
}

[shader("fragment")]
GBufferFragmentOutput fs_main(in VertexOutput input, in bool IsFrontFace : SV_IsFrontFace)
{
	GBufferFragmentOutput output;
	float2 uv = input.uv;

	output.base_color = float4(base_color.Sample(uv).rgb, opacity);
    output.normal = float4(input.world_normal, IsFrontFace ? 1.0 : -1.0);
    output.emissive = float4(emissive, 1.0);
    output.msra = float4(metalic, specular, roughness, ao);
	return output;
}
