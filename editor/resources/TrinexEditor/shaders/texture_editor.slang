#include "trinex/platform.slang"
#include "trinex/quad.slang"

struct VertexOutput
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
}

uniform float4 mask;
uniform Sampler2D texture;
uniform uint mip_level;
uniform float power;

[shader("vertex")]
void vs_main(in uint vertex : SV_VertexID, out VertexOutput out)
{
	out.position = FullScreenQuad::vertex_by_index(vertex);
	out.uv = Platform::validate_uv(FullScreenQuad::uv_by_index(vertex));
}

[shader("fragment")]
float4 fs_main(in VertexOutput input) : SV_Target
{
	float4 out_color = texture.SampleLevel(input.uv, mip_level) * mask;

    if ( mask.a > 0.f && out_color.a < 0.01f )
	{
		discard;
	}

    return float4(pow(out_color.xyz, power), 1.f);
}