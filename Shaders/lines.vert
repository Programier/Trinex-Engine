#version 430 core
layout(location = 0) in vec3 v_position;

uniform mat4 projview;
uniform mat4 model;
mat4 get_model()
{
    if (model == mat4(0))
        return mat4(1);
    return model;
}


void main()
{
    gl_Position = projview * get_model() * vec4(v_position, 1.0);
}
