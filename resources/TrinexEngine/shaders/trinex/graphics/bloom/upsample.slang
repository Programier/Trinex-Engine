import "trinex/quad.slang";
import "trinex/trinex.slang";

struct Args
{
	float2 offset;
	float2 size;
	float weight;
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
	const int radius = 1;

	float3 a = scene.SampleLevel(uv, 0.f, int2(-radius, radius)).rgb;
	float3 b = scene.SampleLevel(uv, 0.f, int2(0, radius)).rgb;
	float3 c = scene.SampleLevel(uv, 0.f, int2(radius, radius)).rgb;

	float3 d = scene.SampleLevel(uv, 0.f, int2(-radius, 0)).rgb;
	float3 e = scene.SampleLevel(uv, 0.f, int2(0, 0)).rgb;
	float3 f = scene.SampleLevel(uv, 0.f, int2(radius, 0)).rgb;

	float3 g = scene.SampleLevel(uv, 0.f, int2(-radius, -radius)).rgb;
	float3 h = scene.SampleLevel(uv, 0.f, int2(0, -radius)).rgb;
	float3 i = scene.SampleLevel(uv, 0.f, int2(radius, -radius)).rgb;

	float3 color = ((e * 4.0) + ((b + d + f + h) * 2.0) + (a + c + g + i)) / 16.f;
	return float4(color * args.weight, 1.f);
}
