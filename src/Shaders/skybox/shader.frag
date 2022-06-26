#version 430 core

in vec3 view_dir;
uniform samplerCube cubemap;

out vec4 color;



void main()
{
    color = texture(cubemap, view_dir);
}
