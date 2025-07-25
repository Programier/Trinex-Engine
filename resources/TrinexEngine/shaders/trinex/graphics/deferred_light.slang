import "trinex/lighting.slang";
import "trinex/attributes.slang";
import "trinex/scene_view.slang";
import "trinex/quad.slang";

uniform Sampler2D base_color_texture;
uniform Sampler2D normal_texture;
uniform Sampler2D msra_texture;
uniform Sampler2D depth_texture;

[parameter_flags(meta::UniformBuffer)]
uniform Lighting::LightParameters parameters;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")] 
float4 fragment_main(in float4 position : SV_Position) : SV_Target
{
	float2 uv = scene_view.screen_uv(position.xy);

	Lighting::PrimitiveData primitive;
	float depth          = depth_texture.Sample(uv).r;

	primitive.base_color = base_color_texture.Sample(uv);
	primitive.position   = scene_view.reconstruct_position(position.xy, depth);
	primitive.normal     = normal_texture.Sample(uv).xyz;
	primitive.msra       = msra_texture.Sample(uv);

	return Lighting::calculate_light(parameters, primitive) * Lighting::LightType::shadow(primitive.position, parameters);
}
