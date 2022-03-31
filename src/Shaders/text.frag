#version 430 core
uniform vec3 color;
uniform sampler2D text;
out vec4 out_color;
in vec2 coords;

void main()
{
    out_color = vec4(color, 1) * vec4(1.0, 1.0, 1.0, texture(text, coords).r);
}
