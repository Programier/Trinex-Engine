import "trinex/light.slang";
import "trinex/math.slang";

namespace Lighting
{
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

		static float3 direction(in float3 world_light_vector, in LightParameters params)
        {
            return normalize(world_light_vector);
        }
    }
}