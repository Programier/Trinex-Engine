import "trinex/trinex.slang";

struct VS_INPUT
{
	float2 pos : POSITION;
	float2 uv : TEXCOORD;
	float4 col : COLOR;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float4 col : COLOR0;
};

uniform float4x4 projection;
uniform Sampler2D texture;

[shader("vertex")]
PS_INPUT vertex_main(in VS_INPUT input)
{
	PS_INPUT output;
	output.pos = projection * float4(input.pos.xy, 0.f, 1.f);
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

[shader("fragment")]
float4 fragment_main(in PS_INPUT input) : SV_Target
{
    return input.col * texture.Sample(input.uv); 
}
