#version 320 es
precision mediump float;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 pos;
void main()
{
    gl_Position = vec4(aPos, 1.f);
    pos = vec2(aTexCoord);
}