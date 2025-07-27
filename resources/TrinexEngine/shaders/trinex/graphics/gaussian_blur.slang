struct GaussianBlur<Value : IFloat>
{
	Value calculate(float sigma, float radius, IFunc<Value, float> sampler)
	{
		
    }
}


import "trinex/trinex.slang";
import "trinex/quad.slang";

uniform Texture2D source;
uniform SamplerState sampler;

struct Args
{
	float2 offset;
	float2 inv_size;
	float2 direction;
	float sigma;
	float radius;
};

[parameter_flags(meta::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
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
		value += source.SampleLevel(sampler, uv + i * args.inv_size, 0.f) * weight;
		weight_sum += weight;
	}

	return value / weight_sum;
}