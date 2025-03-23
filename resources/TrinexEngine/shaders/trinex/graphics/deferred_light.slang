#include "trinex/light.slang"

[parameter_type("CombinedSurface")]
uniform Sampler2D base_color_texture;
[parameter_type("CombinedSurface")]
uniform Sampler2D normal_texture;
[parameter_type("CombinedSurface")]
uniform Sampler2D emissive_texture;
[parameter_type("CombinedSurface")]
uniform Sampler2D msra_texture;
[parameter_type("CombinedSurface")]
uniform Sampler2D depth_texture;

#if TRINEX_SHADOWED_LIGHTING_PASS
[parameter_type("CombinedSurface")]
uniform Sampler2D shadow_map_texture;
uniform float4x4 shadow_map_projview;
#endif

uniform Lighting::LightData light_data;

[shader("vertex")] 
float4 vs_main(in uint vertex : SV_VertexID, out float2 uv : TEXCOORD0, out float2 ndc : TEXCOORD1) : SV_Position
{
	float2 min_v = globals.viewport.xy / globals.size;
	float2 max_v = (globals.viewport.xy + globals.viewport.zw) / globals.size;
	uv           = Platform::validate_uv((FullScreenQuad::uv_by_index(vertex) * (max_v - min_v)) + min_v);

	float4 vertex = FullScreenQuad::vertex_by_index(vertex);
	ndc           = vertex.xy;
	return vertex;
}

float3 reconstruct_position(in float2 ndc, in float2 uv)
{
	float depth           = depth_texture.Sample(uv).r;
	float4 clip_space_pos = float4(ndc, lerp(Platform::ndc_depth_range.x, Platform::ndc_depth_range.y, depth), 1.0);
	float4 view_space_pos = mul(globals.inv_projview, clip_space_pos);
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

#if TRINEX_SHADOWED_LIGHTING_PASS
	float4 shadow_ndc = mul(shadow_map_projview, float4(primitive.position, 1.0));
	shadow_ndc.xyz /= shadow_ndc.w;

	primitive.shadow_depth    = shadow_ndc.z;
	primitive.primitive_depth = shadow_map_texture.Sample(Platform::validate_uv(shadow_ndc.xy * 0.5 + 0.5)).r;
#endif

	return Lighting::calculate_light(light_data, primitive);
}
