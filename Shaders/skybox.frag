#version 430 core

out vec4 FragColor;

in vec3 TexCoords;
uniform int light = 0;

uniform samplerCube skybox;
uniform vec2 lol;

void main()
{
    vec4 light_vector = vec4(1, 1, 1, 1);
    if(light == 1)
        light_vector = vec4(0.1, 0.1, 0.1, 1);
    FragColor = light_vector * texture(skybox, TexCoords);
}
