#include "common.slang"

#if USE_LINES_FOR_GBUFFER
typedef GBufferFragmentOutput FragmentOutput;
#else
struct FragmentOutput
{
	float4 base_color : SV_TARGET0;
};
#endif

[shader("vertex")]
float4 vs_main(	in float3 position : POSITION0, in float4 color : COLOR0, out float4 out_position : SV_Position) : COLOR0
{
	out_position = mul(globals.projview, float4(position, 1.0));
	return color;
}

[shader("fragment")]
FragmentOutput fs_main(in float4 color : COLOR0, in float4 position : SV_Position) : SV_Target
{
	FragmentOutput output;
	output.base_color = float4(color.rgb, 1.0);
#if USE_LINES_FOR_GBUFFER
	output.position = position;
    output.normal   = float4(0.0);
    output.emissive = float4(0.0);
    output.msra     = float4(0.0, 0.0, 0.0, 1.0);
#endif
	return output;
}