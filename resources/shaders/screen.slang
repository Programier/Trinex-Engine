#include "platform.slang"

Sampler2D screen_texture;

[shader("vertex")]
float2 vs_main(in float3 position : POSITION0, out float4 out_position : SV_Position) : TEXCOORD0
{
	out_position = float4(position, 1.0);
	return platform_validate_uv((float2(position.x, position.y) * 0.5) + float2(0.5, 0.5));
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