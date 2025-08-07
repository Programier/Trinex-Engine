
// 552 bytes
struct Cluster
{
	static const uint s_lights_per_cluster = 128;
	static const float3 s_grid_size        = float3(16, 9, 24);

	float4 min;
	float4 max;

	uint point_lights;
	uint spot_lights;
	uint light_indices[s_lights_per_cluster];
}
