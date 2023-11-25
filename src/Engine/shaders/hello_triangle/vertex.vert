#version 320 es
precision highp float;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec4 out_color;

layout(binding = 0) uniform Global
{
    mat4 projview;
    vec2 size;
    float time;
    float dt;
    float min_depth;
    float max_depth;
} global;


void main()
{
    gl_Position = global.projview * vec4(in_position.xyz, 1.0);
    out_position = in_position;
    out_color    = in_color;
}
