#include "common.slang"

struct VertexInput
{
    nointerpolation float3 position: POSITION0;
};

struct VertexOutput
{
    float4 position: SV_Position;
    float3 world_position: POSITION0;
    float3 camera_xz : POSITION1;
};

struct FragmentOutput
{
    float4 color = float4(1.0);
};


uniform float4 color;
uniform float scale;

[shader("vertex")]
VertexOutput vertex_main(VertexInput input)
{
    VertexOutput output;
    float3 camera_xz = float3(globals.camera_location.x, 0, globals.camera_location.z);
    float3 new_position = (input.position + floor(camera_xz / scale)) * scale;
    
    output.position = mul(globals.projview, float4(new_position, 1.0));
    output.world_position = new_position;
    output.camera_xz = camera_xz;

    return output;
}

[shader("fragment")]
FragmentOutput fragment_main(VertexOutput input) : SV_Target
{
    FragmentOutput output;
    float3 view_direction = normalize(globals.camera_location - input.world_position);
    float3 view_direction_xz = normalize(input.camera_xz - input.world_position);

    float factor = 1.0 - pow(dot(view_direction, view_direction_xz), 18);
    output.color = float4(color.rgb * float3(factor * color.w), 1.0);
    return output;
}
