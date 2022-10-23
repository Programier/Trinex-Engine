#version 320 es
precision mediump float;

uniform sampler2D texture0;

in vec2 coord;
out vec4 color;

uniform float power;

void main()
{
    color = vec4(vec3(pow(texture(texture0, coord).r, power)), 1.f);
}
