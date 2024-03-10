struct VertexInput
{
    float3 position: POSITION0;
};

struct VertexOutput
{
    float3 position: SV_Position;
};

struct FragmentOutput
{
    float4 color;
    float4 position;
};


[shader("vertex")]
VertexOutput vertex_main(VertexInput input)
{
    VertexOutput output;
    output.position = input.position;
    return output;
}

[shader("fragment")]
FragmentOutput fragment_main(VertexOutput input) : SV_Target
{
    FragmentOutput output;
    output.color = float4(0.0);
    output.position = float4(0.0);
    return output;
}
