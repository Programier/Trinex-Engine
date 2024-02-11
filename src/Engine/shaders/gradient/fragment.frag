#version 310 es
precision highp float;

layout(location = 0) out vec4 out_color;

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
    float time;
    float delta_time;
    float fov;
    float gamma;
    float aspect_ratio;
}
global;

void main()
{
    out_color = vec4(0.5 + 0.5 * sin(global.time), 0.5 + 0.5 * cos(global.time), 0.5 + 0.5 * sin(global.time + global.delta_time), 1.0);
}
