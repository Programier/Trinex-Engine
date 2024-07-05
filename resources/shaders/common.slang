#ifndef COMMON_SLANG
#define COMMON_SLANG

struct GlobalParameters
{
    float4x4 projection;
    float4x4 view;
    float4x4 projview;
    float4x4 inv_projview;

    float4 viewport;

    float3 camera_location;
    float3 camera_forward;
    float3 camera_right;
    float3 camera_up;

    float2 size;
    float2 depth_range;

    float gamma;
    float time;
    float delta_time;

    float fov;
    float ortho_width;
    float ortho_height;
    float near_clip_plane;
    float far_clip_plane;
    float aspect_ratio;
    int camera_projection_mode;
};

[vk::binding(0, 0)]
ConstantBuffer<GlobalParameters> globals; 

struct GBufferFragmentOutput
{
    [vk::location(0)] float4 base_color   : SV_TARGET0;
    [vk::location(1)] float4 position     : SV_TARGET1;
    [vk::location(2)] float4 normal       : SV_TARGET2;
    [vk::location(3)] float4 emissive     : SV_TARGET3;
    [vk::location(4)] float4 msra         : SV_TARGET4;
};
#endif