#ifndef TRINEX_RANDOM_SLANG
#define TRINEX_RANDOM_SLANG 1

namespace Random
{
	float pseudo_random(float2 xy)
	{
		float2 p = frac(xy / 128.0f) * 128.0f + float2(-64.340622f, -72.465622f);
		return frac(dot(p.xyx * p.xyy, float3(20.390625f, 60.703125f, 2.4281209f)));
	}

	float fast_random(uint2 xy, float magic = 3571.0)
	{
		float2 random2 = (1.0 / 4320.0) * xy + float2(0.25, 0.0);
		float random   = frac(dot(random2 * random2, magic));
		return frac(random * random * (2 * magic));
	}

	uint wang_hash(uint seed)
	{
		seed = (seed ^ 61u) ^ (seed >> 16);
		seed *= 9u;
		seed = seed ^ (seed >> 4);
		seed *= 0x27d4eb2du;
		seed = seed ^ (seed >> 15);
		return seed;
	}

	float wang_random(uint seed)
	{
		return frac(wang_hash(seed) * (1.0 / 4294967296.0));
	}

	float wang_random(uint2 xy)
	{
		uint seed = xy.x * 73856093u ^ xy.y * 19349663u;
		return wang_random(seed);
	}

	float noise(float n)
	{
		return frac(sin(n) * 43758.5453123);
	}

	float2 noise(float2 n)
	{
		return frac(sin(dot(n, float2(12.9898, 78.233))) * float2(43758.5453123, 28001.8384));
	}

	float perlin_ramp(float t)
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

	float2 perlin_ramp(float2 t)
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

	float3 perlin_ramp(float3 t)
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

	float4 perlin_ramp(float4 t)
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}
}// namespace Random

#endif
