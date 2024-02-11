#version 310 es
precision highp float;

layout(location = 0) out vec4 out_color;

#ifndef CAMERA_TYPE_PERSPECTIVE
#define CAMERA_TYPE_PERSPECTIVE 0
#endif

#ifndef CAMERA_TYPE_ORTHOGRAPHIC
#define CAMERA_TYPE_ORTHOGRAPHIC 1
#endif

layout(binding = 0, std140) uniform _Global
{
    layout(column_major) mat4 projection;
    layout(column_major) mat4 view;

    layout(column_major) mat4 projview;
    layout(column_major) mat4 inv_projview;

    vec4 viewport;
    vec4 scissor;

    vec3 camera_location;
    vec3 camera_forward;
    vec3 camera_right;
    vec3 camera_up;

    vec2 size;
    vec2 depth_range;

    float gamma;
    float time;
    float delta_time;

    float fov;
    float ortho_width;
    float ortho_height;
    float near_clip_plane;
    float far_clip_plane;
    float aspect_ratio;
    int camera_type;
}
global;

layout(binding = 1, std140) uniform _Local
{
    vec3 offset;
}
local;


void main()
{
    out_color = vec4(0.5 + 0.5 * sin(global.time + local.offset.x), 0.5 + 0.5 * cos(global.time + local.offset.x),
                     0.5 + 0.5 * sin(global.time + global.delta_time + local.offset.x), 1.0);
}
