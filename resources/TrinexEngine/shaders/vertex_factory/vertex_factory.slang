#ifndef VERTEX_FACTORY
#define VERTEX_FACTORY

interface IVertexFactory
{
	float3 	get_position();
	float3 	get_normal();
	float3 	get_tangent();
	float3 	get_binormal();
	float4 	get_color();
	float2 	get_uv(uint index);
	uint	get_uv_count();

	float4 get_world_position();
	float3 get_world_normal();
	float3 get_world_tangent();
	float3 get_world_binormal();
};

#endif