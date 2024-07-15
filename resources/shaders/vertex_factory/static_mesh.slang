#include "vertex_factory/vertex_factory.slang"
#include "vertex_factory/local_to_world.slang"
#include "math.slang"

#ifndef DISABLE_TANGENT_AND_BINORMAL
#define DISABLE_TANGENT_AND_BINORMAL 1
#endif

struct VertexFactory : IVertexFactory
{
	float3 	position		: POSITION0;
	float2 	uv				: TEXCOORD0;
	float3	normal			: NORMAL0;

#if !DISABLE_TANGENT_AND_BINORMAL
	float3	tangent			: TANGENT0;
	float3	binormal		: BINORMAL0;
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
#if DISABLE_TANGENT_AND_BINORMAL
		return float3(1.0, 0.0, 0.0);
#else
		return tangent;
#endif
	}

	float3 get_binormal()
	{
#if DISABLE_TANGENT_AND_BINORMAL
		return float3(0.0, 0.0, 1.0);
#else
		return binormal;
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

	float4 get_world_position()
	{
		return mul(local_to_world, float4(position, 1.0));
	}

	float3 get_world_normal()
	{
		return rotate_vector(normal, local_to_world);
	}

	float3 get_world_tangent()
	{
		return rotate_vector(get_tangent(), local_to_world);
	}

	float3 get_world_binormal()
	{
		return rotate_vector(get_binormal(), local_to_world);
	}

	uint	get_uv_count()
	{
		return 1;
	}
};