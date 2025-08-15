import "trinex/quad.slang";
import "trinex/trinex.slang";

struct VertexOutput
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
}

extern float4 texture_view(float2 uv);

uniform float4x4 transform;
uniform float4 mask;
uniform float2 range;

[shader("vertex")]
void vertex_main(in uint vertex : SV_VertexID, out VertexOutput out)
{
	out.position = transform * FullScreenQuad::vertex_by_index(vertex);
	out.uv = FullScreenQuad::uv_by_index(vertex);
}

[shader("fragment")]
float4 fragment_main(in VertexOutput input) : SV_Target
{
	float4 color = texture_view(input.uv) * mask;
	color        = (color - range.x) * range.y;
	return color;
}
