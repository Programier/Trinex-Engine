struct GlobalParameters
{
    float4x4 projection;
    float4x4 view;
    float4x4 projview;
    float4x4 inv_projview;

    float4 viewport;
    float4 scissor;

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
