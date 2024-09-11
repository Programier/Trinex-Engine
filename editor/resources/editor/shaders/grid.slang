#include "common.slang"

#if AXIS_SHADER 
    #define GRID_AXIS_CODE(A, B) B
#else
    #define GRID_AXIS_CODE(A, B) A
#endif

struct VertexInput
{
    float3 position: POSITION0;
};

struct VertexOutput
{
    float4 position: SV_Position;
    float3 world_position: POSITION0;
    float3 camera_xz : POSITION1;
};


uniform GRID_AXIS_CODE(float4, float3) color;
GRID_AXIS_CODE(, uniform float3 offset;)
uniform float scale;

[shader("vertex")]
VertexOutput vs_main(VertexInput input)
{
    VertexOutput output;
    float3 camera_xz = float3(globals.camera_location.x, 0, globals.camera_location.z);
    float3 new_position = ((input.position GRID_AXIS_CODE(+ floor(camera_xz / scale), )) * scale) GRID_AXIS_CODE(, + offset);
    
    output.position = mul(globals.projview, float4(new_position, 1.0));
    output.world_position = new_position;
    output.camera_xz = camera_xz;

    return output;
}

[shader("fragment")]
float4 fs_main(VertexOutput input) : SV_Target
{
    float3 view_direction = normalize(globals.camera_location - input.world_position);
    float3 view_direction_xz = normalize(input.camera_xz - input.world_position);

    float factor = 1.0 - pow(abs(dot(view_direction, view_direction_xz)), 18);
    return float4(color.rgb * float3(factor GRID_AXIS_CODE(* color.w, )), 1.0);
}
