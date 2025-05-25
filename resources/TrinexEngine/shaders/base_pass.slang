#include "trinex/attributes.slang"
#include "trinex/common.slang"
#include "trinex/math.slang"
#include "trinex/globals.slang"

[parameter_type("LocalToWorld")]
uniform float4x4 local_to_world;

uniform Sampler2D base_color;
uniform float3 emissive;
uniform float opacity;
uniform float metalic;
uniform float specular;
uniform float roughness;
uniform float ao;


struct VertexFactory
{
	float3 	position		: POSITION0;
	float2 	uv				: TEXCOORD0;
	float3	normal			: NORMAL0;

	float3 get_world_position()
	{
		float4 result = mul(local_to_world, float4(position, 1.0));
		result /= result.w;
		return result.xyz;
	}

	float3 get_world_normal()
	{
		return rotate_vector(normal, local_to_world);
	}

	float2 get_uv()
	{
		return uv;
	}
};

struct VertexOutput
{
	float4 position 		: SV_Position;
#if TRINEX_GEOMETRY_PASS
	float4 world_position 	: POSITION0;
	float3 world_normal 	: NORMAL0;
	float2 uv 				: TEXCOORD0;
#endif
};

struct DepthOutput
{
};

#if TRINEX_GEOMETRY_PASS
typedef GBufferFragmentOutput FragmentOutput;
#else
typedef DepthOutput FragmentOutput;
#endif


[shader("vertex")]
VertexOutput vs_main(in VertexFactory input)
{
	VertexOutput output;

	float4 world_position = float4(input.get_world_position(), 1.0);
	output.position = mul(globals.projview, world_position);

#if TRINEX_GEOMETRY_PASS
	output.world_position = world_position;
	output.world_normal = input.get_world_normal();
	output.uv =	input.get_uv();
#endif

	return output;
}

[shader("fragment")]
FragmentOutput fs_main(in VertexOutput input, in bool IsFrontFace : SV_IsFrontFace)
{
	FragmentOutput output;

#if TRINEX_GEOMETRY_PASS
	float2 uv = input.uv;
	output.base_color = float4(base_color.Sample(uv).rgb, opacity);
    output.normal = float4(input.world_normal, IsFrontFace ? 1.0 : -1.0);
    output.emissive = float4(emissive, 1.0);
    output.msra = float4(metalic, specular, roughness, ao);
#endif

	return output;
}
