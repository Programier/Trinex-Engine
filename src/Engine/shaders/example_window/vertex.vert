#version 320 es
precision highp float;


vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);


void main()
{
    gl_Position = vec4(positions[gl_VertexID].xy, 0.0, 1.0);
}
