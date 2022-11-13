#pragma once
#include <string>

namespace Engine
{
	const std::string DepthRender_shader_frag = R"***(#version 320 es
precision mediump float;

uniform sampler2D texture0;

in vec2 coord;
out vec4 color;

uniform float power;

void main()
{
    color = vec4(vec3(pow(texture(texture0, coord).r, power)), 1.f);
}
)***";



	const std::string DepthRender_shader_vert = R"***(#version 320 es
precision mediump float;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 coord;

void main()
{
    gl_Position = vec4(aPos, 1.f);
    coord = aTexCoord;
}
)***";



	const std::string main_frag = R"***(#version 320 es
precision mediump float;

out vec4 f_color;

in vec3 normal;
in vec2 texture_coords;
in vec3 pixel;
in vec4 FragPosLightSpace;

uniform sampler2D texture0;  // Diffuse
uniform sampler2D texture1;  // Shadow
uniform samplerCube texture2;// SkyBox

uniform vec3 camera;
uniform int lighting = 0;
uniform int RenderType;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


struct Light {
    int type;// 0 - point, 1 - sun, 2 - spot, 3 - area
    vec3 direction;
    float max_distance;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int pixels_block;
};

uniform Material material;
uniform Light light;

vec4 get_texture_color()
{
    return texture(texture0, texture_coords);
}


float get_shadow(vec3 lightDir, vec3 viewDir)
{

    vec2 texelSize = 1.0 / textureSize(texture1, 0);
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.f || projCoords.x > 1.f || projCoords.y < 0.f || projCoords.y > 1.f)
        return 1.f;

    float currentDepth = projCoords.z;


    // Calculating bias value
    float bias = abs(dot(normal, -lightDir)) * ((texelSize.y) / 4);

    // Calculating shadows
    vec2 coord_shift = mod(projCoords.xy, texelSize);
    vec2 reversed_shift = texelSize - coord_shift;
    vec2 center = projCoords.xy;

    float shadow = 0.f;
    for (int x = 0; x <= 1; x += 1)
        for (int y = 0; y <= 1; y += 1)
            shadow += currentDepth - bias > texture(texture1, center + vec2(x, y) * texelSize).r ? 1.f : 0.0;
    return shadow / 4;
}


vec4 get_color()
{
    vec3 result = vec3(1.f);
    if (lighting == 1)
    {
        // ambienta
        vec3 ambient = light.ambient * material.ambient;

        vec3 lightDir = light.type == 1 ? normalize(light.direction) : normalize(pixel - light.position);
        vec3 viewDir = normalize(pixel - camera);


        float coef = 1.f;
        if (dot(lightDir, normal) * dot(normal, viewDir) < 0.f)
            coef = 0.f;

        // diffuse
        vec3 diffuse = light.diffuse * (abs(dot(normal, lightDir)) * material.diffuse * coef);

        // specular

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(abs(dot(normal, halfwayDir)), material.shininess);
        vec3 specular = light.specular * (spec * material.specular * coef);

        float shadow = get_shadow(lightDir, viewDir);
        result = ambient + (1.f - shadow) * (diffuse + specular);
    }


    result *= vec3(get_texture_color());
    return vec4(result, 1.f);
}

void main()
{
    if (RenderType == 0)
    {
        f_color = vec4(0.f);
        return;
    }

    if (RenderType == 1)// Scene Render
    {
        f_color = get_color();
        return;
    }


    if (RenderType == 3)// Depth Buffer
    {
        float depthValue = texture(texture1, texture_coords).r;
        depthValue = pow(depthValue, (light.type == 1 ? 1.f : 20.f));

        f_color = vec4(vec3(depthValue), 1.f);
    }
}
)***";



	const std::string depthBuffer_frag = R"***(#version 320 es
precision mediump float;

void main()
{

}
)***";



	const std::string frame_shader_frag = R"***(#version 320 es
precision mediump float;

uniform sampler2D texture0;

in vec2 pos;
out vec4 color;


void main()
{

    color = texture(texture0, pos);
}
)***";



	const std::string frame_shader_vert = R"***(#version 320 es
precision mediump float;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 pos;
void main()
{
    gl_Position = vec4(aPos, 1.f);
    pos = vec2(aTexCoord);
}
)***";



	const std::string scene_shader_frag = R"***(#version 320 es
precision highp float;

out vec4 f_color;

in vec3 normal;
in vec2 texture_coords;
in vec3 pixel;
in vec4 FragPosLightSpace;

uniform sampler2D texture0;// Diffuse
uniform sampler2D texture1;// Shadow

uniform vec3 camera;
uniform int lighting;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


struct Light {
    int type;// 0 - point, 1 - sun, 2 - spot, 3 - area
    vec3 direction;
    float max_distance;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int pixels_block;
};

uniform Material material;
uniform Light light;

vec4 get_texture_color()
{
    return texture(texture0, texture_coords);
}


float get_shadow(vec3 lightDir, vec3 viewDir)
{

    vec2 texelSize = 1.0 / vec2(textureSize(texture1, 0));
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.f || projCoords.x > 1.f || projCoords.y < 0.f || projCoords.y > 1.f)
        return 1.f;

    float currentDepth = projCoords.z;


    // Calculating bias value
    float bias = abs(dot(normal, -lightDir)) * ((texelSize.y) / 4.f);

    // Calculating shadows
    vec2 coord_shift = mod(projCoords.xy, texelSize);
    vec2 reversed_shift = texelSize - coord_shift;
    vec2 center = projCoords.xy;

    float shadow = 0.f;
    for (int x = 0; x <= 1; x += 1)
    {
        for (int y = 0; y <= 1; y += 1)
        {
            shadow += currentDepth - bias > texture(texture1, center + vec2(x, y) * texelSize).r ? 1.f : 0.0;
        }
    }
    return float(shadow / 4.f);
}

void main()
{
    vec3 result = vec3(1.f);
    if (lighting == 1)
    {
        // ambient
        vec3 ambient = light.ambient * material.ambient;
        vec3 lightDir = light.type == 1 ? normalize(light.direction) : normalize(pixel - light.position);
        vec3 viewDir = normalize(pixel - camera);


        float coef = 1.f;
        if (dot(lightDir, normal) * dot(normal, viewDir) < 0.f)
            coef = 0.f;

        // diffuse
        vec3 diffuse = light.diffuse * (abs(dot(normal, lightDir)) * material.diffuse * coef);

        // specular

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(abs(dot(normal, halfwayDir)), material.shininess);
        vec3 specular = light.specular * (spec * material.specular * coef);

        float shadow = get_shadow(lightDir, viewDir);
        result = ambient + (1.f - shadow) * (diffuse + specular);
    }

    f_color = vec4(result * vec3(get_texture_color()), 1.f);
}
)***";



	const std::string scene_shader_vert = R"***(#version 320 es
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
)***";



	const std::string text_shader_frag = R"***(#version 320 es
precision mediump float;
uniform vec4 color;
uniform sampler2D text;
out vec4 out_color;
in vec2 coords;

void main()
{
    out_color = color * vec4(1.0, 1.0, 1.0, texture(text, coords).r); //
}
)***";



	const std::string text_shader_vert = R"***(#version 320 es
precision mediump float;
layout (location = 0) in vec4 vertex;
out vec2 coords;
uniform mat4 projview;

void main()
{
    gl_Position = projview * vec4(vertex.xy, 0.0, 1.0);
    coords = vertex.zw;
}
)***";



	const std::string skybox_shader_frag = R"***(#version 320 es
precision mediump float;
in vec3 view_dir;
uniform samplerCube cubemap;

out vec4 color;



void main()
{
    color = texture(cubemap, view_dir);
}
)***";



	const std::string skybox_shader_vert = R"***(#version 320 es
precision mediump float;

layout (location = 0) in vec3 v_pos;
out vec3 view_dir;
uniform mat4 projview;

void main()
{
    gl_Position = vec4(projview * vec4(v_pos, 1.f)).xyww;
    view_dir = v_pos;
}
)***";



	const std::string line_shader_frag = R"***(#version 320 es
precision mediump float;

out vec4 out_color;
uniform vec4 color;

void main()
{
    out_color = color;
}
)***";



	const std::string line_shader_vert = R"***(#version 320 es
precision mediump float;
layout(location = 0) in vec3 v_position;

uniform mat4 projview;
uniform mat4 model;

void main()
{
    gl_Position = projview * model * vec4(v_position, 1.0);;
}
)***";



	const std::string lines_frag = R"***(#version 320 es
precision mediump float;

out vec4 out_color;
uniform vec3 color;

in vec3 pixel;
uniform vec3 camera;
uniform int light = 0;

void main()
{
    vec4 K = vec4(1, 1, 1, 1);
    if (light != 0)
    {
        float diff = abs(distance(pixel, camera));
        float max = 100;
        if(diff > max)
            diff = max;
        diff /= max;
        K = vec4(1 - diff, 1 - diff, 1 - diff, 1 - diff);
    }

    out_color = K * vec4(color, 1.0f);
}
)***";



	const std::string depth_shader_frag = R"***(#version 320 es
precision mediump float;

void main()
{
     //gl_FragDepth = gl_FragCoord.z;
}
)***";



	const std::string depth_shader_vert = R"***(#version 320 es
precision mediump float;
layout (location = 0) in vec3 aPos;

uniform mat4 projview;
uniform mat4 model;

void main()
{
    gl_Position = projview * model * vec4(aPos, 1.0);
}
)***";



	const std::string main_vert = R"***(#version 320 es
precision mediump float;
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
)***";



	const std::string depthBuffer_vert = R"***(#version 320 es
precision mediump float;
layout (location = 0) in vec3 aPos;

uniform mat4 projview;
uniform mat4 model;

void main()
{
    gl_Position = projview * model * vec4(aPos, 1.0);
}
)***";



	const std::string lines_vert = R"***(#version 320 es
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
)***";
}
