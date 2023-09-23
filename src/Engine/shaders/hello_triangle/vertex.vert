#version 310 es

precision highp float;
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_colors;

vec3 colors[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

layout(location = 0) out vec3 color;

void main()
{
    gl_Position = vec4(in_position.xy, 0.0, 1.0);
    color = in_colors;
}
