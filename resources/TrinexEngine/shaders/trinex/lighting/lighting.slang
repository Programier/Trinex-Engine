import "trinex/math.slang";
import "trinex/scene_view.slang";

namespace Lighting
{
	interface ILight
	{
		static float attenuation(in float3 world_light_vector, in LightParameters light);
		static float3 direction(in float3 world_light_vector, in LightParameters light);
		static float shadow(in PrimitiveParameters primitive, in LightParameters light, in ByteAddressBuffer shadow_data, SamplerComparisonState sampler);
	}

	struct LightParameters 
	{
		float3 color;
		float3 location;
		float3 direction;
		float2 spot_angles;
		float intensivity;
		float attenuation_radius;
		float inv_attenuation_radius;
		float fall_off_exponent;
		float source_radius;
		uint shadow_address;
	};

	struct LightRanges 
	{
		uint4 point;
		uint4 spot;
		uint4 directional;
	};

	[Flags]
	enum PrimitiveFlags
	{
		Undefined = 0x0,
		IsTranslucent = 0x1
	};

	struct PrimitiveParameters
	{
		float3 base_color;
		float3 location;
		float3 normal;
		float4 msra;
		float depth;
		
		PrimitiveFlags flags;
	};

	struct PointLightShadowParameters
	{
		TextureCube.Handle shadow_map;
		float depth_bias;
		float slope_scale;

		[ForceInline]
		float sample(SamplerComparisonState sampler, float3 dir, float depth)
		{
			return shadow_map.SampleCmpLevelZero(sampler, dir, depth);
		}
	};

	struct SpotLightShadowParameters
	{
		Texture2D.Handle shadow_map;
		float4x4 projview;
		float depth_bias;
		float slope_scale;

		[ForceInline] 
		float sample(SamplerComparisonState sampler, float2 uv, int2 offset, float depth)
		{
			return shadow_map.SampleCmpLevelZero(sampler, uv, depth, offset);
		}
	};

	struct DirectioanlLightShadowCascade
	{
		Texture2D.Handle shadow_map;
		float4x4 projview;

		[ForceInline]
		float sample(SamplerComparisonState sampler, float2 uv, int2 offset, float depth)
		{
			return shadow_map.SampleCmpLevelZero(sampler, uv, depth, offset);
		}
	}

	struct DirectionalLightShadowParameters
	{
		float4 splits;
		float depth_bias;
		float slope_scale;

		[ForceInline]
		uint cascade(float depth)
		{
			float4 count = float4(depth.xxxx >= splits);
			return uint(count.x + count.y + count.z + count.w);
		}
	}
	
	float distribution_ggx(in float3 normal, in float3 halfway_direction, float roughness)
	{
		float roughness4 = Math::square(Math::square(roughness));
		float n_dot_h2   = Math::square(max(dot(normal, halfway_direction), 0.0));
		float denom      = n_dot_h2 * (roughness4 - 1.0) + 1.0;
		return roughness4 / (Math::pi * (denom * denom));
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

	[ForceInline]
	float3 fresnel_schlick(in float cos_theta, in float3 F0)
	{
		return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
	}

	[ForceInline]
	float fresnel_zero_reflectance(in float specular)
	{
		float IOR = lerp(1.3, 1.7, specular);
		return Math::square((IOR - 1.0) / (IOR + 1.0));
	}

	float3 calculate_lighting<LightType : ILight>(in LightParameters light, in PrimitiveParameters primitive)
	{
		float3 view_direction  = normalize(scene_view.camera.location - primitive.location);
		float3 light_direction = light.location.rgb - primitive.location;
		float attenuation      = LightType.attenuation(light_direction, light);
		light_direction        = LightType.direction(light_direction, light);

		float3 normal = primitive.normal;

		if (primitive.flags & PrimitiveFlags::IsTranslucent)
		{
			normal = normal * sign(dot(normal, light_direction));
		}

		float3 f0 = lerp(float3(fresnel_zero_reflectance(primitive.msra.g)), primitive.base_color, float3(primitive.msra.r));

		float normal_dot_l = max(dot(normal, light_direction), 0.0);

		float3 halfway_direction = normalize(view_direction + light_direction);
		float3 radiance          = light.color * light.intensivity * attenuation;

		float ggx       = distribution_ggx(normal, halfway_direction, primitive.msra.b);
		float geometry  = geometry_smith(normal, halfway_direction, light_direction, primitive.msra.b);
		float3 fresnel  = fresnel_schlick(max(dot(halfway_direction, view_direction), 0.0), f0);
		float3 specular = (ggx * geometry * fresnel) / (4.0 * max(dot(normal, view_direction), 0.0) * normal_dot_l + 0.001);

		float3 k_s = fresnel;
		float3 k_d = float3(1.0, 1.0, 1.0) - k_s;
		k_d *= 1.0 - primitive.msra.r;

		float3 reflectance = (k_d * primitive.base_color / Math::pi + specular) * radiance * normal_dot_l;
		return reflectance;
	}
}// namespace Lighting
