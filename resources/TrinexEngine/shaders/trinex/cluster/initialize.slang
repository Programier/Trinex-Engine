import "trinex/trinex.slang";
import "trinex/scene_view.slang";
import "trinex/cluster/cluster.slang";

uniform RWStructuredBuffer<Cluster> clusters;

[ForceInline]
float3 screen_to_view(float2 coord)
{
	float4 ndc = float4(coord * scene_view.viewport.inv_size * 2.0 - 1.0, 0.0, 1.0);
	float4 view_coord = scene_view.camera.inv_projection * ndc;
	return view_coord.xyz / view_coord.w;
}

[ForceInline]
float3 intersection(float3 direction, float z)
{
	return (z / direction.z) * direction;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 cluster_id: SV_GroupID)
{
	float3 cluster = float3(cluster_id);

	float2 tile_size = scene_view.viewport.size / Cluster::s_grid_size.xy;

	float3 min_tile = screen_to_view(cluster.xy * tile_size);
	float3 max_tile = screen_to_view((cluster.xy + 1) * tile_size);

	float slice_near = scene_view.camera.near * pow(scene_view.camera.far / scene_view.camera.near, cluster.z / Cluster::s_grid_size.z);
	float slice_far = scene_view.camera.near * pow(scene_view.camera.far / scene_view.camera.near, (cluster.z + 1) / Cluster::s_grid_size.z);

	float3 min_point_near = intersection(min_tile, slice_near);
	float3 min_point_far  = intersection(min_tile, slice_far);
	float3 max_point_near = intersection(max_tile, slice_near);
	float3 max_point_far  = intersection(max_tile, slice_far);

	uint cluster_index = cluster_id.x + cluster_id.y * uint(Cluster::s_grid_size.x) + cluster_id.z * uint(Cluster::s_grid_size.x) * uint(Cluster::s_grid_size.y);

	clusters[cluster_index].min = float4(min(min(min_point_near, min_point_far), min(max_point_near, max_point_far)), 0.f);
	clusters[cluster_index].max = float4(max(max(min_point_near, min_point_far), max(max_point_near, max_point_far)), 0.f);
}
