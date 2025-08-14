import "trinex/trinex.slang";
import "trinex/quad.slang";

uniform Sampler2D source;

struct Args
{
	uint4 swizzle;// [0 -> R, 1 -> G, 2 -> B, 3 -> A, 4 -> 0.f, 5 -> 1.f]
	float2 offset;
	float2 inv_size;
	float2 direction;
	float sigma;
	float radius;
};

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[ForceInline]
float4 calculate_swizzle(in float4 color)
{
	const float swizzle[6] = {
		color.r,
		color.g,
		color.b,
		color.a,
		0.f,
		1.f
	};

	return float4(swizzle[args.swizzle.r], swizzle[args.swizzle.g], swizzle[args.swizzle.b], swizzle[args.swizzle.a]);
}

[shader("fragment")]
float4 fragment_main(in float4 position: SV_Position) : SV_Target
{
	float2 uv = args.offset + position.xy * args.inv_size;

	float4 value = float4(0.f);
	float weight_sum = 0.f;

	for (float i = -args.radius; i <= args.radius; i++)
	{
		float weight = exp(-0.5f * (i * i) / (args.sigma * args.sigma)) / (2.50662827f * args.sigma);
		value += source.SampleLevel(uv + i * args.inv_size, 0.f) * weight;
		weight_sum += weight;
	}

	return calculate_swizzle(value / weight_sum);
}