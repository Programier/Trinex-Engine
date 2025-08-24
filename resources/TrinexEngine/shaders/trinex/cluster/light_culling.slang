import "trinex/cluster/cluster.slang";
import "trinex/lighting/lighting.slang";
import "trinex/scene_view.slang";
import "trinex/trinex.slang";

uniform RWStructuredBuffer<Cluster> clusters;
uniform StructuredBuffer<Lighting::LightParameters> lights;

[parameter_type(meta::type::UniformBuffer)]
uniform Lighting::LightRanges ranges;

[ForceInline]
float3 make_view_space_location(float3 world_space)
{
	float4 view_space = scene_view.camera.view * float4(world_space, 1.f);
	return view_space.xyz / view_space.w;
}

[ForceInline]
float3 make_view_space_direction(float3 world_space)
{
	return normalize(float3x3(scene_view.camera.view) * world_space);
}

bool point_light_intersection(uint light_index, float3 min, float3 max)
{
	Lighting::LightParameters light = lights[light_index];

	float3 light_location = make_view_space_location(light.location);

	float3 closest = clamp(light_location, min, max);
	float distance = dot(closest - light_location, closest - light_location);
	return distance <= light.attenuation_radius * light.attenuation_radius;
}

bool spot_light_intersection(uint light_index, float3 min, float3 max)
{
	Lighting::LightParameters light = lights[light_index];

	float3 light_location  = make_view_space_location(light.location);
	float3 light_direction = make_view_space_direction(light.direction);

	float3 closest         = clamp(light_location, min, max);
	float distance_squared = dot(closest - light_location, closest - light_location);
	float radius_squared   = light.attenuation_radius * light.attenuation_radius;

	if (distance_squared > radius_squared)
		return false;

	float3 box_center       = 0.5f * (min + max);
	float3 box_half_extents = 0.5f * (max - min);
	float box_radius        = length(box_half_extents);

	float3 light_to_box = box_center - light_location;
	
	float distance_to_box = length(light_to_box);
	
	if (distance_to_box < 1e-6f)
		return true;

	light_to_box /= distance_to_box;

	float cos_a = light.spot_angles.x;
	float sin_a = sqrt(saturate(1.0f - cos_a * cos_a));

	float sin_b = saturate(box_radius / distance_to_box);
	float cos_b = sqrt(saturate(1.0f - sin_b * sin_b));

	float cos_a_plus_b = cos_a * cos_b - sin_a * sin_b;

	return dot(light_direction, light_to_box) >= cos_a_plus_b;
}

[shader("compute")]
[numthreads(128, 1, 1)]
void compute_main(uint3 thread : SV_DispatchThreadID)
{
	Cluster cluster = clusters[thread.x];

	cluster.point_lights = { 0, 0 };
	cluster.spot_lights  = { 0, 0 };

	for (uint i = ranges.point.x; i < ranges.point.y; ++i)
	{
		if (point_light_intersection(i, cluster.min.xyz, cluster.max.xyz))
		{
			cluster.light_indices[cluster.point_lights.x++] = i;
		}
	}

	cluster.point_lights.y = cluster.point_lights.x;

	for (uint i = ranges.point.z; i < ranges.point.w; ++i)
	{
		if (point_light_intersection(i, cluster.min.xyz, cluster.max.xyz))
		{
			cluster.light_indices[cluster.point_lights.y++] = i;
		}
	}

	cluster.spot_lights.x = cluster.point_lights.y;

	for (uint i = ranges.spot.x; i < ranges.spot.y; ++i)
	{
		if (spot_light_intersection(i, cluster.min.xyz, cluster.max.xyz))
		{
			cluster.light_indices[cluster.spot_lights.x++] = i;
		}
	}

	cluster.spot_lights.y = cluster.spot_lights.x;

	for (uint i = ranges.spot.z; i < ranges.spot.w; ++i)
	{
		if (spot_light_intersection(i, cluster.min.xyz, cluster.max.xyz))
		{
			cluster.light_indices[cluster.spot_lights.y++] = i;
		}
	}

	clusters[thread.x] = cluster;
}