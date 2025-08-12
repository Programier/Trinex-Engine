import "trinex/lighting/lighting.slang";
import "trinex/lighting/light_types.slang";
import "trinex/trinex.slang";
import "trinex/scene_view.slang";
import "trinex/quad.slang";
import "trinex/cluster/cluster.slang";

uniform DescriptorHandle<Texture2D> base_color_texture;
uniform DescriptorHandle<Texture2D> normal_texture;
uniform DescriptorHandle<Texture2D> msra_texture;
uniform DescriptorHandle<Texture2D> depth_texture;
uniform DescriptorHandle<SamplerState> sampler;

uniform DescriptorHandle<StructuredBuffer<Cluster>> clusters;
uniform StructuredBuffer<Lighting::LightParameters> lights;

[parameter_flags(meta::UniformBuffer)]
uniform Lighting::LightRanges ranges;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[ForceInline]
uint calculate_cluster_index(in float3 world_space, in float2 coord)
{
	float4 view_space = scene_view.view * float4(world_space, 1.0);
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

	Lighting::PrimitiveData primitive;
	float depth          = depth_texture.Sample(sampler, uv).r;

	primitive.base_color = base_color_texture.Sample(sampler, uv);
	primitive.position   = scene_view.reconstruct_position(position.xy, depth);
	primitive.normal     = normal_texture.Sample(sampler, uv).xyz;
	primitive.msra       = msra_texture.Sample(sampler, uv);

	float3 color = float3(0.f);
	
	// Render directional lights
	for (uint i = ranges.directional.x; i < ranges.directional.y; ++i)
	{
		color += Lighting::calculate_light<Lighting::DirectionalLight>(lights[i], primitive);
	}

	uint cluster_index = calculate_cluster_index(primitive.position, position.xy);
	Cluster cluster    = clusters[cluster_index];

	for (uint i = 0; i < cluster.point_lights; ++i)
	{
		uint index = cluster.light_indices[i];
		color += Lighting::calculate_light<Lighting::PointLight>(lights[index], primitive);
	}

	for (uint i = 0; i < cluster.spot_lights; ++i)
	{
		uint index = cluster.light_indices[cluster.point_lights + i];
		color += Lighting::calculate_light<Lighting::SpotLight>(lights[index], primitive);
	}

	return float4(color, 1.f);
}
