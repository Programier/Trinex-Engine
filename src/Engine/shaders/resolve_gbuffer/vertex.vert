#version 310 es
precision highp float;
layout(location = 0) in vec2 in_position;
layout(location = 0) out vec2 out_position;

void main()
{
    gl_Position = vec4(in_position.x, in_position.y, 0.0, 1.0f);
    out_position = in_position;
}
