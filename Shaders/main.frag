#version 430 core

in vec4 a_color;

in vec3 texture_coords;
in vec3 pixel;
out vec4 f_color;

uniform sampler2DArray textures;
uniform vec3 camera;
uniform int light = 1;

vec4 get_texture_color()
{
    return texture(textures, vec3(texture_coords));
}

vec4 get_color()
{
    if (light == 0)
        return get_texture_color();

    float max = 100;
    float diff = abs(distance(pixel, camera));
    if (diff > max)
        return vec4(0, 0, 0, 1);
    diff /= max;
    return vec4(1 - diff, 1 - diff, 1 - diff, 1) * get_texture_color();
}

void main()
{
    f_color = get_color();
}
