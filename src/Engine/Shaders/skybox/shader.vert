#version 320 es
precision mediump float;

layout (location = 0) in vec3 v_pos;
out vec3 view_dir;
uniform mat4 projview;

void main()
{
    gl_Position = vec4(projview * vec4(v_pos, 1.f)).xyww;
    view_dir = v_pos;
}
