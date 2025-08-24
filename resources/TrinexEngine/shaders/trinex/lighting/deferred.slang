import "trinex/lighting/lighting.slang";
import "trinex/lighting/light_types.slang";
import "trinex/trinex.slang";
import "trinex/scene_view.slang";
import "trinex/quad.slang";
import "trinex/cluster/cluster.slang";

uniform Texture2D base_color_texture;
uniform Texture2D normal_texture;
uniform Texture2D msra_texture;
uniform Texture2D depth_texture;

uniform SamplerState screen_sampler;
uniform SamplerComparisonState shadow_sampler;

uniform StructuredBuffer<Cluster> clusters;
uniform StructuredBuffer<Lighting::LightParameters> lights;
uniform ByteAddressBuffer shadows;

[parameter_type(meta::type::UniformBuffer)]
uniform Lighting::LightRanges ranges;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[ForceInline]
uint calculate_cluster_index(in float3 location, in float2 coord)
{
	float4 view_space = scene_view.camera.view * float4(location, 1.0);
	float depth = view_space.z / view_space.w;

	float near = scene_view.camera.near;
	float far  = scene_view.camera.far;
	
	float log_near_far = log(scene_view.camera.far / scene_view.camera.near);

	uint slice       = uint((log(abs(depth) / near) * Cluster::s_grid_size.z) / log(far / near));
	float2 tile_size = scene_view.viewport.size / Cluster::s_grid_size.xy;

	uint3 tile = uint3(uint2(coord / tile_size), slice);
	return tile.x + (tile.y * uint(Cluster::s_grid_size.x)) + (tile.z * uint(Cluster::s_grid_size.x) * uint(Cluster::s_grid_size.y));
}


[shader("fragment")] 
float4 fragment_main(in float4 position : SV_Position) : SV_Target
{
	float2 uv = scene_view.screen_uv(position.xy);

	Lighting::PrimitiveParameters primitive;
	primitive.depth      = depth_texture.Sample(screen_sampler, uv).r;
	primitive.base_color = base_color_texture.SampleLevel(screen_sampler, uv, 0.f);
	primitive.location   = scene_view.reconstruct_position(position.xy, primitive.depth);
	primitive.normal     = normal_texture.SampleLevel(screen_sampler, uv, 0.f).xyz;
	primitive.msra       = msra_texture.SampleLevel(screen_sampler, uv, 0.f);

	float3 color = float3(0.f);

	for (uint i = ranges.directional.x; i < ranges.directional.y; ++i)
	{
		color += Lighting::calculate_light<Lighting::DirectionalLight>(lights[i], primitive);
	}

	for (uint i = ranges.directional.z; i < ranges.directional.w; ++i)
	{
		Lighting::LightParameters light = lights[i];
		float shadow = Lighting::DirectionalLight::shadow(primitive, light, shadows, shadow_sampler);
		color += Lighting::calculate_light<Lighting::DirectionalLight>(light, primitive) * shadow;
	}

	uint cluster_index = calculate_cluster_index(primitive.location, position.xy);
	Cluster cluster    = clusters[cluster_index];

	uint i = 0;

	for (;i < cluster.point_lights.x; ++i)
	{
		uint index = cluster.light_indices[i];
		color += Lighting::calculate_light<Lighting::PointLight>(lights[index], primitive);
	}

	for (; i < cluster.point_lights.y; ++i)
	{
		uint index                      = cluster.light_indices[i];
		Lighting::LightParameters light = lights[index];
		float shadow                    = Lighting::PointLight::shadow(primitive, light, shadows, shadow_sampler);
		color += Lighting::calculate_light<Lighting::PointLight>(light, primitive) * shadow;
	}

	for (; i < cluster.spot_lights.x; ++i)
	{
		uint index = cluster.light_indices[i];
		color += Lighting::calculate_light<Lighting::SpotLight>(lights[index], primitive);
	}

	for (; i < cluster.spot_lights.y; ++i)
	{
		uint index                      = cluster.light_indices[i];
		Lighting::LightParameters light = lights[index];
		float shadow                    = Lighting::SpotLight::shadow(primitive, light, shadows, shadow_sampler);
		color += Lighting::calculate_light<Lighting::SpotLight>(light, primitive) * shadow;
	}

	return float4(color, 1.f);
}
