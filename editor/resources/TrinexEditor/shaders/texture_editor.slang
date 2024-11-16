struct VertexInput
{
	float3 position : POSITION0;	
};

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
void vs_main(in VertexInput input, out VertexOutput out)
{
	out.position = float4(input.position.x, input.position.y, input.position.z, 1.f);
	out.uv = (input.position.xy + float2(1.f, 1.f)) * float2(0.5f, 0.5f);
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