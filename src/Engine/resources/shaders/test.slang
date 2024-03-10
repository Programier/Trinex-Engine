struct VertexInput
{
    float3	position : POSITION0;
    float3	color    : COLOR0;
    float3	color2   : COLOR1;
};

struct VertexOutput
{
    float4          position     : SV_Position;
};

uniform float4x4 mpv;

[shader("vertex")]
VertexOutput vertex_main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(mpv, float4(0.0));
    return output;
}
