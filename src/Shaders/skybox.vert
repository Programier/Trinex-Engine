#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projview;



void main()
{
    TexCoords = aPos;
    gl_Position = vec4(projview * vec4(aPos, 1.0)).xyww;
}
