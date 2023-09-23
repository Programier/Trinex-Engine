#version 310 es
precision highp float;

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(color.rgb, 1.0);
}

