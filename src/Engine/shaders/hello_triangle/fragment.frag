#version 310 es
precision highp float;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;
layout(location = 3) out vec3 out_specular;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

#define VALUE 0.99

layout(binding = 1, set = 0) uniform Global
{
    mat4 projview;
    vec2 viewport;
    float time;
    float dt;
} global;

vec3 get_normal()
{
    if(abs(in_position.x) >= VALUE)
        return vec3(in_position.x, 0.0, 0.0);
    if(abs(in_position.y) >= VALUE)
        return vec3(0.0, in_position.y, 0.0);
    if(abs(in_position.z) >= VALUE)
        return vec3(0.0, 0.0, in_position.z);
    return vec3(0.0);
}

void main()
{

    float x = abs(tan(global.time));
    float y = abs(cos(global.time));
    float z = max(x, y);

    vec4 tmp = vec4(x, y, z, 1.0);

    out_color    = mix(in_color, tmp, 0.0);
    out_position = in_position;
    out_normal   = abs(get_normal());
    out_specular = vec3(1.0f);
}