import "trinex/trinex.slang";
import "trinex/quad.slang";

uniform Sampler2D source;

struct Arguments
{
	float2 offset;
	float2 inv_size;
	uint4 swizzle;// [0 -> R, 1 -> G, 2 -> B, 3 -> A, 4 -> 0.f, 5 -> 1.f]
};

[parameter_flags(meta::UniformBuffer)]
uniform Arguments args;

[ForceInline]
static float4 calculate_swizzle(in float4 color)
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

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float4 position: SV_Position) : SV_Target
{
	float2 uv    = args.offset + position.xy * args.inv_size;
	float4 color = source.SampleLevel(uv, 0);
	return calculate_swizzle(color);
}