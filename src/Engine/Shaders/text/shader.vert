#version 320 es
precision mediump float;
layout (location = 0) in vec4 vertex;
out vec2 coords;
uniform mat4 projview;

void main()
{
    gl_Position = projview * vec4(vertex.xy, 0.0, 1.0);
    coords = vertex.zw;
}
