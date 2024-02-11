#version 310 es
precision highp float;

layout(location = 0) in vec2 in_coords;

#ifndef PROJECTION_MODE_PERSPECTIVE
#define PROJECTION_MODE_PERSPECTIVE 0
#endif

#ifndef PROJECTION_MODE_ORTHOGRAPHIC
#define PROJECTION_MODE_ORTHOGRAPHIC 1
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
    int camera_projection_mode;
} global;


void main(void)
{
    gl_Position = global.projview * vec4(in_coords.xy, 0.0, 1.0);
}
