#version 310 es
precision highp float;

layout(location = 0) in vec2 in_position;

layout(binding = 2) buffer MyBuffer {
    int size;
    int data[];
};

void main()
{
    if(size > 1)
    {
        gl_Position = vec4(in_position.xy, 0.0, 1.0);
    }
    else
    {
        gl_Position = vec4(in_position.xy, 0.0, 1.0);
    }
}
