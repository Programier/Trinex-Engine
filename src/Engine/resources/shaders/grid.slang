#include "common.slang"

struct VertexInput
{
    float3 position: POSITION0;
};

struct VertexOutput
{
    float4 position: SV_Position;
};

struct FragmentOutput
{
    float4 color = float4(1.0);
};

[shader("vertex")]
VertexOutput vertex_main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(globals.projview, float4(input.position, 1.0));
    return output;
}

[shader("fragment")]
FragmentOutput fragment_main(VertexOutput input) : SV_Target
{
    FragmentOutput output;
    output.color = float4(1.0, 0.0, 0.0, 1.0);
    return output;
}
