import "trinex/trinex.slang";

struct VSInput
{
	float2 position : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
}

struct VSOutput
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
}

uniform float2 translate;
uniform float4x4 transform;

[shader("vertex")]
VSOutput vertex_main(VSInput input)
{
	VSOutput output;

	output.position = transform * float4(input.position + translate, 0.0, 1.0);
	output.color    = input.color;
	output.uv       = input.uv;
	
	return output;
}

[shader("fragment")]
float4 fragment_main(VSOutput input) : SV_Target
{
	return input.color;
}