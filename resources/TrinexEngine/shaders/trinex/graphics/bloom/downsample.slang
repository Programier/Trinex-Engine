import "trinex/quad.slang";
import "trinex/trinex.slang";

uniform Sampler2D scene;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID, out float2 uv: TEXCOORD0) : SV_Position
{
	uv = FullScreenQuad::uv_by_index(index);
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float2 uv: TEXCOORD0) : SV_Target
{
	const int2 offsets[13] = { 
		int2(-2, 2), int2(0, 2), int2(2, 2),
		int2(-2, 0), int2(0, 0), int2(2, 0),
		int2(-2, -2), int2(0, -2), int2(2, -2),
		int2(-1, 1), int2(1, 1), int2(-1, -1),
		int2(1, -1)
	};

	const float weights[13] = { 
		0.03125, 0.0625,  0.03125, 0.0625, 0.125, 0.0625, 0.03125, 0.0625,  0.03125, 0.125, 0.125, 0.125, 0.125
	};

	float4 result = float4(0.f);

	[unroll]
	for (int i = 0; i < 13; ++i)
		result += scene.SampleLevel(uv, 0.f, offsets[i]) * weights[i];

	return result;
}
