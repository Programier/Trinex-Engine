import "trinex/trinex.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";
import "trinex/cluster/cluster.slang";
import "trinex/lighting/lighting.slang";
import "trinex/lighting/light_types.slang";

namespace Lighting
{
    [parameter_type(meta::type::ExcludeMaterialParameter)]
	uniform SamplerComparisonState shadow_sampler;
	[parameter_type(meta::type::ExcludeMaterialParameter)]
	uniform StructuredBuffer<Cluster> clusters;
	[parameter_type(meta::type::ExcludeMaterialParameter)]
	uniform StructuredBuffer<Lighting::LightParameters> lights;
	[parameter_type(meta::type::ExcludeMaterialParameter)]
	uniform ByteAddressBuffer shadows;

	[parameter_type(meta::type::UniformBuffer | meta::type::ExcludeMaterialParameter)]
	uniform Lighting::LightRanges ranges;

	[ForceInline]
	static uint calculate_cluster_index(in float3 location, in float2 coord)
	{
		float4 view_space = scene_view.camera.view * float4(location, 1.0);
		float depth       = view_space.z / view_space.w;

		float near = scene_view.camera.near;
		float far  = scene_view.camera.far;

		float log_near_far = log(scene_view.camera.far / scene_view.camera.near);

		uint slice       = uint((log(abs(depth) / near) * Cluster::s_grid_size.z) / log(far / near));
		float2 tile_size = scene_view.viewport.size / Cluster::s_grid_size.xy;

		uint3 tile = uint3(uint2(coord / tile_size), slice);
		return tile.x + (tile.y * uint(Cluster::s_grid_size.x)) + (tile.z * uint(Cluster::s_grid_size.x) * uint(Cluster::s_grid_size.y));
	}

	float3 gather_lighting(in PrimitiveParameters primitive, float2 screen, float3 color = float3(0.f))
	{
		for (uint i = ranges.directional.x; i < ranges.directional.y; ++i)
		{
			color += Lighting::calculate_lighting<Lighting::DirectionalLight>(lights[i], primitive);
		}

		for (uint i = ranges.directional.z; i < ranges.directional.w; ++i)
		{
			Lighting::LightParameters light = lights[i];
			float shadow                    = Lighting::DirectionalLight::shadow(primitive, light, shadows, shadow_sampler);
			color += Lighting::calculate_lighting<Lighting::DirectionalLight>(light, primitive) * shadow;
		}

		uint cluster_index = calculate_cluster_index(primitive.location, screen);
		Cluster cluster    = clusters[cluster_index];

		uint i = 0;

		for (; i < cluster.point_lights.x; ++i)
		{
			uint index = cluster.light_indices[i];
			color += Lighting::calculate_lighting<Lighting::PointLight>(lights[index], primitive);
		}

		for (; i < cluster.point_lights.y; ++i)
		{
			uint index                      = cluster.light_indices[i];
			Lighting::LightParameters light = lights[index];
			float shadow                    = Lighting::PointLight::shadow(primitive, light, shadows, shadow_sampler);
			color += Lighting::calculate_lighting<Lighting::PointLight>(light, primitive) * shadow;
		}

		for (; i < cluster.spot_lights.x; ++i)
		{
			uint index = cluster.light_indices[i];
			color += Lighting::calculate_lighting<Lighting::SpotLight>(lights[index], primitive);
		}

		for (; i < cluster.spot_lights.y; ++i)
		{
			uint index                      = cluster.light_indices[i];
			Lighting::LightParameters light = lights[index];
			float shadow                    = Lighting::SpotLight::shadow(primitive, light, shadows, shadow_sampler);
			color += Lighting::calculate_lighting<Lighting::SpotLight>(light, primitive) * shadow;
		}

		return color;
	}
}