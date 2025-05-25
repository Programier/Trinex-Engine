struct VertexInput
{
    float2 position : POSITION0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

[shader("vertex")]
VertexOutput vs_main(in VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 0.0f, 1.0f);
    output.uv = (input.position + float2(1.f, 1.f)) / float2(2.f, 2.f);
    return output;
}

[shader("fragment")]
float4 fs_main(VertexOutput input) : SV_Target
{
    return float4(input.uv, 0.f, 1.0f);
}