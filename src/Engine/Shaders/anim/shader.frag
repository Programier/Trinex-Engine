#version 320 es
precision highp float;

out vec4 f_color;

in vec3 normal;
in vec2 texture_coords;
in vec3 pixel;



uniform sampler2D texture0;// Diffuse
uniform vec3 camera;

vec4 get_diffuse_color()
{
    return texture(texture0, texture_coords);
}

void main()
{
    vec3 result = vec3(abs(dot(normalize(camera - pixel), normal)));

    vec4 texture_color = get_diffuse_color();

    f_color = vec4(result * vec3(texture_color), texture_color.a);
}
