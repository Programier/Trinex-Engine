import "trinex/lighting.slang";

namespace Lighting
{
	export struct LightType : ILight 
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

		static float shadow(in float3 position, in LightParameters params) { return 1.f; }
	}
}
