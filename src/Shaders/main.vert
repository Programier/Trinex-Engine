#version 430 core
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texture_coords;
layout(location = 2) in vec3 v_normals;


uniform int RenderType = 0;



out vec2 texture_coords;
out vec3 pixel;
out vec3 normal;
out vec4 FragPosLightSpace;

flat out int F_RenderType;

uniform mat4 projview;
uniform mat4 model;
uniform mat4 light_projview;

mat4 get_model()
{
    if (model == mat4(0))
        return mat4(1);
    return model;
}

void main()
{
    F_RenderType = RenderType;
    if(RenderType == 0)
        return;

    if (RenderType == 1) // SceneRender
    {
        texture_coords = v_texture_coords;
        mat4 object_model = get_model();
        vec4 _pixel = object_model * vec4(v_position, 1.0);
        pixel = vec3(_pixel);
        gl_Position = projview * _pixel;
        normal = normalize(mat3(transpose(inverse(model))) * v_normals);
        FragPosLightSpace = light_projview * _pixel;
        return;
    }

    if(RenderType == 3) // Depth Buffer
    {
        gl_Position = projview * model * vec4(v_position, 1.f);
        texture_coords = v_texture_coords;
        return;
    }
}
