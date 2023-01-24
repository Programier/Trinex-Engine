#version 320 es
precision highp float;
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texture_coords;
layout(location = 2) in vec3 v_normals;


struct SSBO_DATA
{
    vec3 mult;
};

layout(binding = 1) buffer SSBO
{
    SSBO_DATA data[];
};

out vec2 texture_coords;
out vec3 pixel;
out vec3 normal;
out vec4 FragPosLightSpace;
out vec3 mult_by;


uniform mat4 projview;
uniform mat4 model;
uniform mat4 light_projview;
uniform mat3 transposed_inversed_model;


vec4 get_vertex_position()
{
    return model * vec4(v_position, 1.0);
}


void main()
{
    texture_coords = v_texture_coords;
    vec4 _pixel = get_vertex_position();

    pixel = vec3(_pixel);
    gl_Position = projview * _pixel;
    normal =  normalize(transposed_inversed_model * v_normals);
    FragPosLightSpace = light_projview * _pixel;
    mult_by = data[0].mult;
}
