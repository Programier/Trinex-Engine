void get_sprite_color(in vec4 input_color, out vec3 output_color, out float output_alpha)
{
    if (input_color.a < 0.1)
    {
        discard;
    }

    output_color = input_color.rgb;
    output_alpha = input_color.a;
}
