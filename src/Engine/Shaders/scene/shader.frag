#version 320 es
precision mediump float;

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
    // f_color = vec4(1, 0, 0, 1);
}
