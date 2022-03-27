#version 430 core
layout(location = 0) in vec3 v_position;

uniform mat4 projview;

void main()
{
    gl_Position = projview * vec4(v_position, 1.0);
}
