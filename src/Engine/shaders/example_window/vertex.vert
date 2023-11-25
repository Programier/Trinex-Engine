#version 320 es
precision highp float;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_colors;

layout(location = 0) out vec4 out_colors;


void main()
{
    gl_Position = vec4(in_position, 1.0);

    out_colors = in_colors;
}
