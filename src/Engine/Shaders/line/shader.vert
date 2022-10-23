#version 320 es
precision mediump float;
layout(location = 0) in vec3 v_position;

uniform mat4 projview;
uniform mat4 model;

void main()
{
    gl_Position = projview * (model == mat4(0.f) ? mat4(1.f) : model) * vec4(v_position, 1.0);;
}
