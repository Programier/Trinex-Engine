#include "trinex/globals.slang"
#include "trinex/math.slang"
#include "trinex/platform.slang"
#include "trinex/quad.slang"

namespace Lighting
{
	struct LightData {
		float4 color;
		float intensivity;
#if POINT_LIGHT || SPOT_LIGHT
		float3 location;
		float radius;
		float fall_off_exponent;
#if SPOT_LIGHT
		float3 direction;
		float2 spot_angles;// {Cos outer cone, inv cos cone difference. }
#endif
#else
		float3 direction;
#endif
#if TRINEX_SHADOWED_LIGHTING_PASS
		float depth_bias;
		float slope_scale;
#endif

		float calc_attenuation(float3 world_light_vector)
		{
#if POINT_LIGHT || SPOT_LIGHT
			world_light_vector /= radius;
			float len_sqr     = dot(world_light_vector, world_light_vector);
			float attenuation = pow(1.0f - saturate(len_sqr), fall_off_exponent);

#if SPOT_LIGHT
			float cos_outer_cone          = spot_angles.x;
			float inv_cos_cone_difference = spot_angles.y;
			attenuation *=
					square(saturate((dot(normalize(world_light_vector), -direction) - cos_outer_cone) * inv_cos_cone_difference));
#endif
			return attenuation;
#else
			return 1.0;
#endif
		}
	};

	struct PrimitiveData {
		float4 base_color;
		float3 position;
		float3 normal;
		float4 msra;

#if TRINEX_SHADOWED_LIGHTING_PASS
		float primitive_depth;
		float shadow_depth;
#endif
	};

	float distribution_ggx(in float3 normal, in float3 halfway_direction, float roughness)
	{
		float roughness4 = square(square(roughness));
		float n_dot_h2   = square(max(dot(normal, halfway_direction), 0.0));
		float denom      = n_dot_h2 * (roughness4 - 1.0) + 1.0;
		return roughness4 / (M_PI * (denom * denom));
	}

	float geometry_schlick_ggx(float normal_dot_v, float roughness)
	{
		float r           = (roughness + 1.0);
		float coefficient = (r * r) / 8.0;
		return normal_dot_v / (normal_dot_v * (1.0 - coefficient) + coefficient);
	}

	float geometry_smith(in float3 normal, in float3 halfway_direction, in float3 light_direction, float roughness)
	{
		float normal_dot_v = max(dot(normal, halfway_direction), 0.0);
		float normal_dot_l = max(dot(normal, light_direction), 0.0);
		return geometry_schlick_ggx(normal_dot_l, roughness) * geometry_schlick_ggx(normal_dot_v, roughness);
	}

	float3 fresnel_schlick(in float cos_theta, in float3 F0)
	{
		return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
	}

	float3 normalize_normal(float3 normal, in float3 view)
	{
		normal = normalize(normal);

		if (dot(normal, view) < 0.0)
			normal = -normal;

		return normal;
	}

	float fresnel_zero_reflectance(in float specular)
	{
		float IOR = lerp(1.3, 1.7, specular);
		return square((IOR - 1.0) / (IOR + 1.0));
	}

	float4 calculate_light(in LightData light, in PrimitiveData primitive)
	{
		if (length(primitive.normal) < 0.1)
		{
			return float4(0.0, 0.0, 0.0, 1.0);
		}

		float3 view_direction = normalize(globals.camera_location -
										  primitive.position);// Using view direction from fragment location to camera location
		float3 normal         = normalize_normal(primitive.normal, view_direction);

		float3 f0 = lerp(float3(fresnel_zero_reflectance(primitive.msra.g)), primitive.base_color.rgb, float3(primitive.msra.r));

#if POINT_LIGHT || SPOT_LIGHT
		float3 light_direction = light.location.rgb - primitive.position;// Using light direction from fragment location to light location
		float attenuation = light.calc_attenuation(light_direction);
		light_direction   = normalize(light_direction);
#else
		float3 light_direction = -light.direction;
		float attenuation      = 1.0;
#endif

		float normal_dot_l = max(dot(normal, light_direction), 0.0);

#if TRINEX_SHADOWED_LIGHTING_PASS
		float depth_bias = light.depth_bias + normal_dot_l * light.slope_scale;
		if(primitive.shadow_depth > primitive.primitive_depth + depth_bias)
			return float4(0, 0, 0, 1.0);
#endif

		float3 halfway_direction = normalize(view_direction + light_direction);
		float3 radiance          = light.color.rgb * light.intensivity * attenuation;

		float ggx       = distribution_ggx(normal, halfway_direction, primitive.msra.b);
		float geometry  = geometry_smith(normal, halfway_direction, light_direction, primitive.msra.b);
		float3 fresnel  = fresnel_schlick(max(dot(halfway_direction, view_direction), 0.0), f0);
		float3 specular = (ggx * geometry * fresnel) / (4.0 * max(dot(normal, view_direction), 0.0) * normal_dot_l + 0.001);

		float3 k_s = fresnel;
		float3 k_d = float3(1.0, 1.0, 1.0) - k_s;
		k_d *= 1.0 - primitive.msra.r;

		
		float3 reflectance = (k_d * primitive.base_color.xyz / M_PI + specular) * radiance * normal_dot_l;
		return float4(reflectance, 1.0);
	}
}// namespace Lighting

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
