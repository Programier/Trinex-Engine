#include "trinex/light.slang"
#include "trinex/attributes.slang"

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

struct PackedLightData {
	[per_instance]
	[vertex_offset(0)]
	[vertex_stream(0)]
	float4 param1 : POSITION1;
	
#if TRINEX_POINT_LIGHT
	[per_instance]
	[vertex_offset(16)]
	[vertex_stream(0)]
	float4 param2 : POSITION2;

	[per_instance]
	[vertex_offset(32)]
	[vertex_stream(0)]
	float param3 : POSITION3;
#endif

#if TRINEX_SPOT_LIGHT
	[per_instance]
	[vertex_offset(16)]
	[vertex_stream(0)]
	float4 param2 : POSITION2;

	[per_instance]
	[vertex_offset(32)]
	[vertex_stream(0)]
	float4 param3 : POSITION3;
	
	[per_instance]
	[vertex_offset(48)]
	[vertex_stream(0)]
	float2 param4 : POSITION4;
#endif

#if TRINEX_DIRECTIONAL_LIGHT
	[per_instance]
	[vertex_offset(16)]
	[vertex_stream(0)]
	float3 param2 : POSITION2;
#endif

	float3 color() { return param1.xyz; }
	float intensivity() { return param1.w; }

#if TRINEX_POINT_LIGHT
	float3 location() { return param2.xyz; }
	float radius() { return param2.w; }
	float fall_off_exponent() { return param3; }
#endif

#if TRINEX_SPOT_LIGHT
	float3 direction() { return param2.xyz; }
	float radius() { return param2.w; }
	float3 location() { return param3.xyz; }
	float fall_off_exponent() { return param3.w; }
	float2 spot_angles() { return param4; } 
#endif

#if TRINEX_DIRECTIONAL_LIGHT
	float3 direction() { return param2; }
#endif
};

#if TRINEX_SHADOWED_LIGHTING_PASS
uniform Lighting::LightData light_data;
#endif

[shader("vertex")] 
float4 vs_main(
#if !TRINEX_SHADOWED_LIGHTING_PASS
	in PackedLightData in_packed_data,
	nointerpolation out PackedLightData out_packed_data,
#endif
	in uint vertex : SV_VertexID,
	out float2 uv : TEXCOORD0,
	out float2 ndc : TEXCOORD1) : SV_Position
{
	uv = Platform::validate_uv(globals.normalize_screen_space_uv(FullScreenQuad::uv_by_index(vertex)));

	float4 vertex = FullScreenQuad::vertex_by_index(vertex);
	ndc           = vertex.xy;

#if !TRINEX_SHADOWED_LIGHTING_PASS
	out_packed_data = in_packed_data;
#endif

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
float4 fs_main(
#if !TRINEX_SHADOWED_LIGHTING_PASS
	nointerpolation in PackedLightData packed_data,
#endif
	in float2 uv : TEXCOORD0,
	in float2 ndc : TEXCOORD1) : SV_Target
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

#if !TRINEX_SHADOWED_LIGHTING_PASS
	Lighting::LightData light_data;

	light_data.color = packed_data.color();
	light_data.intensivity = packed_data.intensivity();
#if TRINEX_POINT_LIGHT || TRINEX_SPOT_LIGHT
	light_data.location = packed_data.location();
	light_data.radius = packed_data.radius();
	light_data.fall_off_exponent = packed_data.fall_off_exponent();

#if TRINEX_SPOT_LIGHT
	light_data.direction = packed_data.direction();
	light_data.spot_angles = packed_data.spot_angles();
#endif
#else
	light_data.direction = packed_data.direction();
#endif

#endif

	return Lighting::calculate_light(light_data, primitive);
}
