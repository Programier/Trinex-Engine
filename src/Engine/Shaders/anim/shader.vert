#version 320 es
precision highp float;
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texture_coords;
layout(location = 2) in vec3 v_normals;
layout(location = 3) in ivec4 v_bones_id;
layout(location = 4) in vec4 v_weights;


layout(binding = 1) buffer SSBO
{
     mat4 bones_matrices[];
};

out vec2 texture_coords;
out vec3 pixel;
out vec3 normal;
flat out ivec4 bones_id;
out vec4 weights;

uniform mat4 projview;
uniform mat4 model;
uniform mat3 transposed_inversed_model;

vec4 get_vertex_position()
{
    vec3 result_pos = vec3(0.f);

    for(int i = 0; i < 4; i++)
    {
        if(v_bones_id[i] == -1)
            continue;
        result_pos += mat3(bones_matrices[v_bones_id[i]]) * v_position  * v_weights[i];
    }

    return model * vec4(result_pos.xyz, 1.0);
}


void main()
{
    texture_coords = v_texture_coords;
    vec4 _pixel = get_vertex_position();

    pixel = vec3(_pixel);
    gl_Position = projview * _pixel;
    normal = normalize(transposed_inversed_model * v_normals);
}
