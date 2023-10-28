#version 310 es
precision highp float;

layout(location = 0) in vec3 in_position;
layout(location = 0) out vec3 out_position;


void main()
{
    gl_Position = vec4(in_position.xyz, 1.0);
    out_position = in_position;
}
