#version 430 core

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
