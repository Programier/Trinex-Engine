import "trinex/quad.slang";
import "trinex/scene_view.slang";

uniform Sampler2D base_color;
uniform Sampler2D msra;
uniform float3 ambient_color;

[shader("vertex")] 
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
    return FullScreenQuad::vertex_by_index(index);
}
 
[shader("fragment")]
float4 fragment_main(in float4 position : SV_Position) : SV_Target
{
	float2 uv = scene_view.screen_uv(position.xy);
    return float4(float3(base_color.Sample(uv).rgb) * ambient_color * msra.Sample(uv).a, 1.0);   
}
