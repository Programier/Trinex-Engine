#version 320 es
precision mediump float;

out vec4 out_color;
uniform vec4 color;

void main()
{
    out_color = color;
}
