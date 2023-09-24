#version 310 es
precision highp float;
precision highp sampler;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler test_sampler;
layout(binding = 1) uniform texture2D test_texture;

void main() {

    vec2 coord = gl_FragCoord.rg / vec2(1280, 720);
    vec4 color = texture(sampler2D(test_texture, test_sampler), coord);
    out_color = vec4(color.rgb, 1.0);
}

