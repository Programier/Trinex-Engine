import "trinex/lighting/lighting.slang";
import "trinex/math.slang";
import "trinex/trinex.slang";

namespace Lighting
{
	[ForceInline]
	static float3 calculate_primitive_projection(in float4x4 projview, in float3 location)
	{
		float4 shadow_ndc = projview * float4(location, 1.0);
		shadow_ndc.xyz /= shadow_ndc.w;
		return float3(shadow_ndc.xy * 0.5 + 0.5, shadow_ndc.z);
	}

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

		static float shadow(in PrimitiveParameters primitive, in LightParameters light, in ByteAddressBuffer shadow_data, SamplerComparisonState sampler)
		{
			PointLightShadowParameters parameters = shadow_data.Load<PointLightShadowParameters>(light.shadow_address);

			float3 direction        = primitive.location - light.location;
			static const float near = 0.1;

			const float primitive_depth = Math::unlinearize_depth(calculate_depth(direction), near, light.attenuation_radius);
			const float bias = Math::calculate_bias(parameters.depth_bias, parameters.slope_scale, primitive.normal, normalize(direction));
			float shadow_depth = parameters.sample(sampler, direction, primitive_depth - bias);
			
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

		static float3 direction(in float3 world_light_vector, in LightParameters light) { return normalize(world_light_vector); }

		static float shadow(in PrimitiveParameters primitive, in LightParameters light, in ByteAddressBuffer shadow_data, SamplerComparisonState sampler)
		{
			SpotLightShadowParameters parameters = shadow_data.Load<SpotLightShadowParameters>(light.shadow_address);

			const float3 light_direction      = normalize(primitive.location - light.location);
			const float3 primitive_projection = calculate_primitive_projection(parameters.projview, primitive.location);

			const float bias = Math::calculate_bias(parameters.depth_bias, parameters.slope_scale, primitive.normal, light_direction);
			const float primitive_depth = primitive_projection.z - bias;

			float shadow_depth =
				parameters.sample(sampler, primitive_projection.xy, int2(-1, -1), primitive_depth) +
			    parameters.sample(sampler, primitive_projection.xy, int2(0, -1), primitive_depth) +
				parameters.sample(sampler, primitive_projection.xy, int2(1, -1), primitive_depth) +
			    parameters.sample(sampler, primitive_projection.xy, int2(-1, 0), primitive_depth) +
				parameters.sample(sampler, primitive_projection.xy, int2(0, 0), primitive_depth) +
				parameters.sample(sampler, primitive_projection.xy, int2(1, 0), primitive_depth) +
				parameters.sample(sampler, primitive_projection.xy, int2(-1, 1), primitive_depth) +
				parameters.sample(sampler, primitive_projection.xy, int2(0, 1), primitive_depth) +
				parameters.sample(sampler, primitive_projection.xy, int2(1, 1), primitive_depth);

			return shadow_depth / 9.0;
		}
	}

	struct DirectionalLight : Lighting::ILight 
	{
		static float attenuation(in float3 world_light_vector, in LightParameters params) { return 1.f; }
		static inline float3 direction(in float3 world_light_vector, in LightParameters params) { return -params.direction; }
		
		static float shadow(in PrimitiveParameters primitive, in LightParameters light, in ByteAddressBuffer shadow_data, SamplerComparisonState sampler)
		{
			DirectionalLightShadowParameters parameters = shadow_data.Load<DirectionalLightShadowParameters>(light.shadow_address);

			uint cascade_index = parameters.cascade(primitive.depth);

			if(cascade_index == 4)
				return 1.f;

			uint cascade_offset = sizeof(DirectionalLightShadowParameters) + cascade_index * 72;

			DirectioanlLightShadowCascade cascade = shadow_data.Load<DirectioanlLightShadowCascade>(light.shadow_address + cascade_offset);

			const float3 primitive_projection = calculate_primitive_projection(cascade.projview, primitive.location);

			const float bias = Math::calculate_bias(parameters.depth_bias, parameters.slope_scale, primitive.normal, light.direction);
			const float primitive_depth = primitive_projection.z - bias;

			float shadow_depth =
				cascade.sample(sampler, primitive_projection.xy, int2(-1, -1), primitive_depth) +
			    cascade.sample(sampler, primitive_projection.xy, int2(0, -1), primitive_depth) +
				cascade.sample(sampler, primitive_projection.xy, int2(1, -1), primitive_depth) +
			    cascade.sample(sampler, primitive_projection.xy, int2(-1, 0), primitive_depth) +
				cascade.sample(sampler, primitive_projection.xy, int2(0, 0), primitive_depth) +
				cascade.sample(sampler, primitive_projection.xy, int2(1, 0), primitive_depth) +
				cascade.sample(sampler, primitive_projection.xy, int2(-1, 1), primitive_depth) +
				cascade.sample(sampler, primitive_projection.xy, int2(0, 1), primitive_depth) +
				cascade.sample(sampler, primitive_projection.xy, int2(1, 1), primitive_depth);

			return shadow_depth / 9.0;
		}
	}
}