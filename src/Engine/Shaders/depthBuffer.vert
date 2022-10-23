#version 320 es
precision mediump float;
layout (location = 0) in vec3 aPos;

uniform mat4 projview;
uniform mat4 model;

void main()
{
    gl_Position = projview * model * vec4(aPos, 1.0);
}
