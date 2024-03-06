void get_ambient_only(in vec4 base_color, in vec4 msra, in vec3 ambient_light, out vec3 out_color, out float out_alpha)
{
    out_color = base_color.rgb * ambient_light * msra.a;
    out_alpha = 1.0;
}
