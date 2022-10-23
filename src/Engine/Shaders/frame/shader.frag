#version 320 es
precision mediump float;

uniform sampler2D texture0;

in vec2 pos;
out vec4 color;


void main()
{

    color = texture(texture0, pos);
}
