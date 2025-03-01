#include "trinex/attributes.slang"
#include "trinex/quad.slang"
#include "trinex/platform.slang"
#include "trinex/common.slang"

[is_globals()]
ConstantBuffer<GlobalParameters> globals;

Sampler2D base_color;
Sampler2D msra;
uniform float3 ambient_color;

[shader("vertex")]
float2 vs_main(in uint vertex : SV_VertexID, out float4 out_position : SV_Position) : TEXCOORD0
{
    out_position = FullScreenQuad::vertex_by_index(vertex);
    float2 uv = FullScreenQuad::uv_by_index(vertex);
    float2 min = globals.viewport.xy / globals.size;
    float2 max = (globals.viewport.xy + globals.viewport.zw) / globals.size;

    return Platform::validate_uv((uv * (max - min)) + min);
}


[shader("fragment")]
float4 fs_main(in float2 uv : TEXCOORD0) : SV_Target
{
    return float4(float3(base_color.Sample(uv).rgb) * ambient_color * msra.Sample(uv).a, 1.0);   
}
