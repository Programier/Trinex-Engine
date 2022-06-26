#version 430 core
uniform vec4 color = vec4(1, 1, 1, 1);
uniform sampler2D text;
out vec4 out_color;
in vec2 coords;

void main()
{
    out_color = color * vec4(1.0, 1.0, 1.0, texture(text, coords).r);
}
