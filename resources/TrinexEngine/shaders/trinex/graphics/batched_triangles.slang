import "trinex/scene_view.slang";

[shader("vertex")]
float4 vs_main(	in float3 position : POSITION0, in float4 color : COLOR0, out float4 out_color : COLOR0) : SV_Position
{
	out_color = color;
	return mul(scene_view.projview, float4(position, 1.0));
}

[shader("fragment")]
float4 fs_main(in float4 color : COLOR0) : SV_Target
{
	return float4(color.rgb, 1.0);
}
