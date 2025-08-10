import "trinex/trinex.slang";

struct VS_INPUT
{
    [vertex_stream(0), vertex_offset(0)] float2 pos : TEXCOORD0;
	[vertex_stream(0), vertex_offset(8)] float2 uv : TEXCOORD1;
	[vertex_stream(0), vertex_offset(16)] float4 col : COLOR0;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float4 col : COLOR0;
};

uniform float4x4 model;
uniform DescriptorHandle<Texture2D> texture;
uniform DescriptorHandle<SamplerState> sampler;

[shader("vertex")]
PS_INPUT vertex_main(in VS_INPUT input)
{
    PS_INPUT output;
    output.pos = model * float4(input.pos.xy, 0.f, 1.f);
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

[shader("fragment")]
float4 fragment_main(in PS_INPUT input) : SV_Target
{
    return input.col * texture.Sample(sampler, input.uv); 
}
