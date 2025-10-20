import "trinex/quad.slang";
import "trinex/trinex.slang";
import "trinex/math.slang";

struct Args
{
	float2 offset;
	float2 size;
	float threshold;
	float knee;
	float clamp;
}

uniform Sampler2D scene;

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID, out float2 uv: TEXCOORD0) : SV_Position
{
	uv = args.offset + args.size * FullScreenQuad::uv_by_index(index);
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float2 uv: TEXCOORD0) : SV_Target
{
	float3 color = scene.SampleLevel(uv, 0.f).rgb;
	color        = color / (1.0 + Math::luminance(color));

	int2 inner_offsets[4] = {
		int2(1, 0), int2(-1, 0),
		int2(0, 1), int2(0, -1),
	};

	int2 ring_offsets[8] = {
		int2(1, 1), int2(2, 2),
		int2(-1, -1), int2(-2, -2),
		int2(1, -1), int2(2, -2),
		int2(-1, 1), int2(-2, 2),
	};

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		float3 sample = scene.SampleLevel(uv, 0.f, inner_offsets[i]).rgb;
		sample        = sample / (1.0 + Math::luminance(sample));
		color += sample * 2.0;
	}

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		float3 sample = scene.SampleLevel(uv, 0.f, ring_offsets[i]).rgb;
		sample        = sample / (1.0 + Math::luminance(sample));
		color += sample;
	}

	color /= 16.f;
	color = color * (1.0 + Math::luminance(color));

	float luminance = Math::luminance(color);
	
	float contribution = 0;
	
	if (luminance > args.threshold)
	{
		if (luminance < args.threshold)
		{
			contribution = Math::square((luminance - args.threshold) / args.knee) * 0.5f;
		}
		else
		{
			contribution = luminance - args.threshold;
		}
	}

	return float4(clamp(color * contribution, float3(0.f), float3(args.clamp)), luminance);
}