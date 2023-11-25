#version 320 es
precision highp float;

layout(location = 0) in vec2 in_position;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D scene_texture;

void main()
{
    vec2 coord = (in_position + vec2(1.0, 1.0)) / vec2(2.0, 2.0);
    out_color = vec4(texture(scene_texture, coord));
}
