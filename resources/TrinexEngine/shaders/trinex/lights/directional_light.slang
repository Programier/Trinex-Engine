import "trinex/lighting.slang";

namespace Lighting
{
	export struct LightType : ILight 
	{
		static float attenuation(in float3 world_light_vector, in LightParameters params) { return 1.f; }
		static inline float3 direction(in float3 world_light_vector, in LightParameters params) { return -params.direction; }
		static float shadow(in float3 position, in LightParameters params) { return 1.f; }
	}
}
