#version 320 es
precision highp float;

layout(location = 0) in vec4 in_colors;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = in_colors;
}
