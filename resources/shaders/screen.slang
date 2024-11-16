#include "trinex/quad.slang"
#include "platform.slang"

Sampler2D screen_texture;
uniform float2 min_point;
uniform float2 max_point;

[shader("vertex")]
float2 vs_main(in uint vertex : SV_VertexID, out float4 out_position : SV_Position) : TEXCOORD0
{
	out_position = FullScreenQuad::vertex_by_index(vertex);
	return platform_validate_uv((FullScreenQuad::uv_by_index(vertex) * (max_point - min_point)) + min_point);
}

[shader("fragment")]
float4 fs_main(in float2 uv : TEXCOORD0) : SV_Target
{
	float4 color = screen_texture.Sample(uv);
#if ALPHA_ALWAYS_ONE
	return float4(color.rgb, 1.0);
#else
	return color;
#endif
}
