#version 320 es
precision highp float;
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texture_coords;
layout(location = 2) in vec3 v_normals;



out vec2 texture_coords;
out vec3 pixel;
out vec3 normal;
out vec4 FragPosLightSpace;


uniform mat4 projview;
uniform mat4 model;
uniform mat4 light_projview;
uniform mat3 transposed_inversed_model;

void main()
{
    texture_coords = v_texture_coords;
    vec4 _pixel = model * vec4(v_position, 1.0);

    pixel = vec3(_pixel);
    gl_Position = projview * _pixel;
    normal = normalize(transposed_inversed_model * v_normals);
    FragPosLightSpace = light_projview * _pixel;
}
