#version 310 es
precision highp float;

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec3 in_position;

void main()
{
    out_color = vec4(1.0, 0.0, 0.0, 1.0);
}
