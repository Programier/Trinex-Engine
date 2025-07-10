import "trinex/lighting.slang";
import "trinex/attributes.slang";
import "trinex/scene_view.slang";
import "trinex/quad.slang";
import "trinex/platform.slang";

uniform Sampler2D base_color_texture;
uniform Sampler2D normal_texture;
uniform Sampler2D msra_texture;
uniform Sampler2D depth_texture;

[parameter_flags(meta::MemoryBlock)]
uniform Lighting::LightParameters parameters;

[shader("vertex")] 
float4 vs_main(in uint vertex : SV_VertexID, out float2 uv : TEXCOORD0, out float2 ndc : TEXCOORD1) : SV_Position
{
	uv            = scene_view.screen_uv(FullScreenQuad::uv_by_index(vertex));
	float4 vertex = FullScreenQuad::vertex_by_index(vertex);
	ndc           = vertex.xy;
	return vertex;
}

float3 reconstruct_position(in float2 ndc, in float2 uv)
{
	float depth           = depth_texture.Sample(uv).r;
	float4 clip_space_pos = float4(ndc, lerp(Platform::ndc_depth_range.x, Platform::ndc_depth_range.y, depth), 1.0);
	float4 view_space_pos = mul(scene_view.inv_projview, clip_space_pos);
	return view_space_pos.xyz /= view_space_pos.w;
}

[shader("fragment")] 
float4 fs_main(in float2 uv : TEXCOORD0, in float2 ndc : TEXCOORD1) : SV_Target
{
	Lighting::PrimitiveData primitive;
	primitive.base_color = base_color_texture.Sample(uv);
	primitive.position   = reconstruct_position(ndc, uv);
	primitive.normal     = normal_texture.Sample(uv).xyz;
	primitive.msra       = msra_texture.Sample(uv);

	return Lighting::calculate_light(parameters, primitive) * Lighting::LightType::shadow(primitive.position, parameters);
}
