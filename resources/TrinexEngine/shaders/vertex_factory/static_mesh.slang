#include "vertex_factory/vertex_factory.slang"
#include "vertex_factory/local_to_world.slang"
#include "math.slang"

#ifndef DISABLE_TANGENT_AND_BITANGENT
#define DISABLE_TANGENT_AND_BITANGENT 1
#endif

struct VertexFactory : IVertexFactory
{
	float3 	position		: POSITION0;
	float2 	uv				: TEXCOORD0;
	float3	normal			: NORMAL0;

#if !DISABLE_TANGENT_AND_BITANGENT
	float3	tangent			: TANGENT0;
	float3	bitangent		: BITANGENT0;
#endif

	float3 get_position()
	{
		return position;
	}

	float3 get_normal()
	{
		return normal;
	}

	float3 get_tangent()
	{
#if DISABLE_TANGENT_AND_BITANGENT
		return float3(1.0, 0.0, 0.0);
#else
		return tangent;
#endif
	}

	float3 get_bitangent()
	{
#if DISABLE_TANGENT_AND_BITANGENT
		return float3(0.0, 0.0, 1.0);
#else
		return bitangent;
#endif
	}

	float2 get_uv(uint index)
	{
		return uv;
	}

	float4 get_color()
	{
		return float4(1.0, 1.0, 1.0, 1.0);
	}

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

	float3 get_world_tangent()
	{
		return rotate_vector(get_tangent(), local_to_world);
	}

	float3 get_world_bitangent()
	{
		return rotate_vector(get_bitangent(), local_to_world);
	}

	uint get_uv_count()
	{
		return 1;
	}
};
