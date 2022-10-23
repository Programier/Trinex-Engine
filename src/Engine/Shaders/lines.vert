#version 320 es
precision mediump float;
layout(location = 0) in vec3 v_position;

uniform mat4 projview;
uniform mat4 model;
out vec3 pixel;
mat4 get_model()
{
    if (model == mat4(0))
        return mat4(1);
    return model;
}


void main()
{
    vec4 _coord = get_model() * vec4(v_position, 1.0);
    pixel = vec3(_coord);
    gl_Position = projview * _coord;
}
