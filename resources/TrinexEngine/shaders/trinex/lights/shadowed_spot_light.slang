import "trinex/lighting.slang";
import "trinex/math.slang";

namespace Lighting
{
	uniform Sampler2DShadow shadow_map;
	uniform float4x4 shadow_projview;

	export struct LightType : ILight 
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

		[ForceInline] 
		static float shadow_sample(float2 uv, int2 offset, float depth) {
			return shadow_map.SampleCmp(uv, depth, offset);
		}

		static float shadow(in float3 position, in LightParameters params)
		{
			float4 shadow_ndc = mul(shadow_projview, float4(position, 1.0));
			shadow_ndc.xyz /= shadow_ndc.w;

			float2 uv = shadow_ndc.xy * 0.5 + 0.5;

			static const float bias = 0.001;
			float primitive_depth   = shadow_ndc.z - bias;

			float shadow_depth =
			        shadow_sample(uv, int2(-1, -1), primitive_depth) + shadow_sample(uv, int2(0, -1), primitive_depth) +
			        shadow_sample(uv, int2(1, -1), primitive_depth) + shadow_sample(uv, int2(-1, 0), primitive_depth) +
			        shadow_sample(uv, int2(0, 0), primitive_depth) + shadow_sample(uv, int2(1, 0), primitive_depth) +
			        shadow_sample(uv, int2(-1, 1), primitive_depth) + shadow_sample(uv, int2(0, 1), primitive_depth) +
			        shadow_sample(uv, int2(1, 1), primitive_depth);

			return shadow_depth / 9.0;
		}
	}
}
