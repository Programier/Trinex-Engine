#version 320 es
precision highp float;
layout(location = 0) in vec3 v_position;

uniform mat4 projview;
uniform mat4 model;

void main()
{
    gl_Position = projview * model * vec4(v_position, 1.0);;
}
