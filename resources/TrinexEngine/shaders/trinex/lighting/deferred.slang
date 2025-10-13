import "trinex/lighting/lighting_gather.slang";
import "trinex/lighting/lighting.slang";
import "trinex/scene_view.slang";
import "trinex/quad.slang";
import "trinex/math.slang";

uniform Texture2D base_color_texture;
uniform Texture2D normal_texture;
uniform Texture2D msra_texture;
uniform Texture2D depth_texture;

uniform SamplerState screen_sampler;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")] 
float4 fragment_main(in float4 position : SV_Position) : SV_Target
{
	float2 uv = scene_view.screen_uv(position.xy);

	Lighting::PrimitiveParameters primitive;
	primitive.depth      = depth_texture.Sample(screen_sampler, uv).r;
	primitive.base_color = base_color_texture.SampleLevel(screen_sampler, uv, 0.f).rgb;

	if (!Math::is_fragment_covered(primitive.depth))
	{
		return float4(primitive.base_color, 1.f);
	}

	primitive.location   = scene_view.reconstruct_position(position.xy, primitive.depth);
	primitive.normal     = normal_texture.SampleLevel(screen_sampler, uv, 0.f).xyz * 2.f - 1.f;
	primitive.msra       = msra_texture.SampleLevel(screen_sampler, uv, 0.f);
	primitive.flags      = Lighting::PrimitiveFlags::Undefined;
	
	float3 result = Lighting::gather_lighting(primitive, position.xy);
	return float4(result, 1.f);
}
