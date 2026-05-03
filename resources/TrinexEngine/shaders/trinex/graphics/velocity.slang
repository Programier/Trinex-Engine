import "trinex/quad.slang";
import "trinex/scene_view.slang";
import "trinex/trinex.slang";

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID, out float2 ndc : TEXCOORD0) : SV_Position
{
	float2 uv = FullScreenQuad::uv(index);
	ndc       = uv * 2.f - 1.f;
	return float4(ndc, 0.f, 1.f);
}

[shader("fragment")]
float2 fragment_main(in float2 ndc : TEXCOORD0) : SV_Target
{
	float3 sample   = scene_view.camera.clip_to_world(ndc, 0.5);
	float4 clip     = scene_view.prev_camera.projview * float4(sample, 1.0);
	float2 prev_ndc = clip.xy / clip.w;
	return (ndc - prev_ndc) * 0.5f;
}