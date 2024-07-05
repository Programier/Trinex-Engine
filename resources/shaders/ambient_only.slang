Sampler2D base_color;
Sampler2D msra;
uniform float3 ambient_color;

[shader("vertex")]
float2 vs_main(in float3 position : POSITION0, out float4 out_position : SV_Position) : TEXCOORD0
{
    out_position = float4(position, 1.0);
    return (float2(position.x, position.y) * 0.5) + float2(0.5, 0.5);
}


[shader("fragment")]
float4 fs_main(in float2 uv : TEXCOORD0) : SV_Target
{
    return float4(float3(base_color.Sample(uv).rgb) * ambient_color * msra.Sample(uv).a, 1.0);   
}