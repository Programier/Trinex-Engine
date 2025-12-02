import "trinex/trinex.slang";
import "trinex/quad.slang";
import "trinex/math.slang";

uniform Sampler2D source;

struct Args
{
	float2 offset;
	float2 size;
	float2 direction;
	float sigma;
	float radius;
	uint swizzle;
};

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID, out float2 uv : TEXCOORD0) : SV_Position
{
	uv = args.offset + args.size * FullScreenQuad::uv_by_index(index);
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float2 uv: TEXCOORD0) : SV_Target
{
	float4 value = float4(0.f);
	float weight_sum = 0.f;

	for (float i = -args.radius; i <= args.radius; i++)
	{
		float weight  = exp(-0.5f * (i * i) / (args.sigma * args.sigma)) / (2.50662827f * args.sigma);
		float2 offset = i * args.direction;
		value += source.SampleLevel(uv + offset, 0.f) * weight;
		weight_sum += weight;
	}

	return Math::swizzle(value / weight_sum, args.swizzle);
}