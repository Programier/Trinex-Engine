import "trinex/quad.slang";
import "trinex/trinex.slang";

struct Args
{
	float2 offset;
	float2 size;
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
	float4 result = scene.SampleLevel(uv, 0.f, int2(0, 0)) * 0.25f +
	                scene.SampleLevel(uv, 0.f, int2(-1, -1)) * 0.0625f +
	                scene.SampleLevel(uv, 0.f, int2(1, -1)) * 0.0625f +
					scene.SampleLevel(uv, 0.f, int2(-1, 1)) * 0.0625f +
	                scene.SampleLevel(uv, 0.f, int2(1, 1)) * 0.0625f +
					scene.SampleLevel(uv, 0.f, int2(0, -1)) * 0.125f +
	                scene.SampleLevel(uv, 0.f, int2(-1, 0)) * 0.125f +
					scene.SampleLevel(uv, 0.f, int2(1, 0)) * 0.125f +
	                scene.SampleLevel(uv, 0.f, int2(0, 1)) * 0.125f;
	return result;
}
