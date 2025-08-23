import "trinex/lighting/lighting.slang";
import "trinex/math.slang";
import "trinex/trinex.slang";

namespace Lighting
{
	struct PointLight : ILight 
	{
		static inline float attenuation(in float3 world_light_vector, in LightParameters params)
		{
			const float distance = length(world_light_vector);
			float attenuation    = pow(saturate(1.0 - (distance * params.inv_attenuation_radius)), params.fall_off_exponent);
			attenuation /= max(distance * distance, 0.01);
			return attenuation;
		}

		static inline float3 direction(in float3 world_light_vector, in LightParameters params)
		{
			return normalize(world_light_vector);
		}

		[ForceInline]
		static float calculate_depth(float3 direction)
		{
			direction = abs(direction);
			return max(max(direction.x, direction.y), direction.z);
		}

		static float shadow(in float3 position, in LightParameters params, in ByteAddressBuffer shadow_data, SamplerComparisonState sampler)
		{
			PointLightShadowData data = shadow_data.Load<PointLightShadowData>(params.shadow_address);

			float3 direction    = position - params.location;

			static const float bias = 0.001;
			static const float near = 0.1;
			float primitive_depth   = Math::unlinearize_depth(calculate_depth(direction), near, params.attenuation_radius) - bias;
			float shadow_depth      = data.sample(sampler, direction, primitive_depth);
			return shadow_depth;
		}
	}

	struct SpotLight : ILight 
	{
		static float attenuation(in float3 world_light_vector, in LightParameters params)
		{
			const float distance = length(world_light_vector);
			float attenuation    = pow(saturate(1.0 - (distance * params.inv_attenuation_radius)), params.fall_off_exponent);
			attenuation /= max(distance * distance, 0.01);

			const float cos_theta               = dot(normalize(world_light_vector), -params.direction);
			const float cos_outer_cone          = params.spot_angles.x;
			const float inv_cos_cone_difference = params.spot_angles.y;

			return attenuation * Math::square(saturate((cos_theta - cos_outer_cone) * inv_cos_cone_difference));
		}

		static float3 direction(in float3 world_light_vector, in LightParameters params) { return normalize(world_light_vector); }

		static float shadow(in float3 position, in LightParameters params, in ByteAddressBuffer shadow_data, SamplerComparisonState sampler)
		{
			SpotLightShadowData data = shadow_data.Load<SpotLightShadowData>(params.shadow_address);

			float4 shadow_ndc = data.projview * float4(position, 1.0);
			shadow_ndc.xyz /= shadow_ndc.w;

			float2 uv = shadow_ndc.xy * 0.5 + 0.5;

			static const float bias = 0.001;
			float primitive_depth   = shadow_ndc.z - bias;

			float shadow_depth =
				data.sample(sampler, uv, int2(-1, -1), primitive_depth) +
			    data.sample(sampler, uv, int2(0, -1), primitive_depth) +
				data.sample(sampler, uv, int2(1, -1), primitive_depth) +
			    data.sample(sampler, uv, int2(-1, 0), primitive_depth) +
				data.sample(sampler, uv, int2(0, 0), primitive_depth) +
				data.sample(sampler, uv, int2(1, 0), primitive_depth) +
				data.sample(sampler, uv, int2(-1, 1), primitive_depth) +
				data.sample(sampler, uv, int2(0, 1), primitive_depth) +
				data.sample(sampler, uv, int2(1, 1), primitive_depth);

			return shadow_depth / 9.0;
		}
	}

	struct DirectionalLight : Lighting::ILight 
	{
		static float attenuation(in float3 world_light_vector, in LightParameters params) { return 1.f; }
		static inline float3 direction(in float3 world_light_vector, in LightParameters params) { return -params.direction; }
		static float shadow(in float3 position, in LightParameters params) { return 1.f; }
	}
}