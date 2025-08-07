import "trinex/cluster/cluster.slang";
import "trinex/lighting/lighting.slang";
import "trinex/scene_view.slang";
import "trinex/trinex.slang";

uniform RWStructuredBuffer<Cluster> clusters;
uniform StructuredBuffer<Lighting::LightParameters> lights;

[parameter_flags(meta::UniformBuffer)]
uniform Lighting::LightRanges ranges;

[ForceInline]
float3 to_view_space(float3 world_space)
{
	float4 view_space = scene_view.view * float4(world_space, 1.f);
	return view_space.xyz / view_space.w;
}

bool point_light_intersection(uint light_index, float3 min, float3 max)
{
	Lighting::LightParameters light = lights[light_index];

	float3 location = to_view_space(light.location);

	float3 closest = clamp(location, min, max);
	float distance = dot(closest - location, closest - location);
	return distance <= light.attenuation_radius * light.attenuation_radius;
}

bool spot_light_intersection(uint light_index, float3 min, float3 max)
{
	Lighting::LightParameters light = lights[light_index];

	float3 closest = clamp(light.location, min, max);
	float3 to_closest = closest - light.location;
	float distance    = dot(to_closest, to_closest);

	if (distance > light.attenuation_radius * light.attenuation_radius)
		return false;

	if (length(to_closest) < 1e-5f)
		return true;

	return dot(light.direction, normalize(to_closest)) >= light.spot_angles.x;
}

[shader("compute")]
[numthreads(128, 1, 1)]
void compute_main(uint3 thread : SV_DispatchThreadID)
{
	Cluster cluster = clusters[thread.x];

	cluster.point_lights = 0;
	cluster.spot_lights  = 0;

	for (uint i = ranges.point.x; i < ranges.point.y; ++i)
	{
		if (point_light_intersection(i, cluster.min.xyz, cluster.max.xyz))
		{
			cluster.light_indices[cluster.point_lights] = i;
			++cluster.point_lights;
		}
	}

	for (uint i = ranges.spot.x; i < ranges.spot.y; ++i)
	{
		if (spot_light_intersection(i, cluster.min.xyz, cluster.max.xyz))
		{
			cluster.light_indices[cluster.point_lights + cluster.spot_lights] = i;
			++cluster.spot_lights;
		}
	}

	clusters[thread.x] = cluster;
}