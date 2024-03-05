
#define M_PI 3.141592
float calc_attenuation(in float radius, in float distance)
{
    float value = clamp(1.0 - pow(distance / radius, 4.0), 0.0, 1.0);
    return pow(value, 2.0) / (pow(distance, 2.0) + 1.0);
}

float distribution_ggx(in vec3 normal, in vec3 view_light_direction, float roughness)
{
    float roughness4    = pow(roughness, 4.0);
    float normal_dot_h2 = pow(max(dot(normal, view_light_direction), 0.0), 2.0);
    float denom         = normal_dot_h2 * (roughness4 - 1.0) + 1.0;
    return roughness4 / (M_PI * pow(denom, 2.0));
}

float geometry_schlick_ggx(float normal_dot_v, float roughness)
{
    float coefficient = pow(roughness + 1.0, 2.0) / 8.0;
    return normal_dot_v / (normal_dot_v * (1.0 - coefficient) + coefficient);
}

float geometry_smith(in vec3 normal, in vec3 view_light_direction, in vec3 light_direction, float roughness)
{
    float normal_dot_v = max(dot(normal, view_light_direction), 0.0);
    float normal_dot_l = max(dot(normal, light_direction), 0.0);
    return geometry_schlick_ggx(normal_dot_l, roughness) * geometry_schlick_ggx(normal_dot_v, roughness);
}

vec3 fresnel_schlick(in float cos_theta, in vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}


void get_point_light(in vec4 base_color, in vec4 position, in vec4 normal, in vec4 emissive, in vec4 msra, in vec3 light_color,
                     in float light_radius, in float light_intensivity, in vec3 light_location, in vec3 ambient_color,
                     out vec3 light_out_color, out float light_out_alpha)
{
    if (length(normal.xyz) < 0.1)
    {
        light_out_color = base_color.rgb * ambient_color;
    }
    else
    {
        vec3 f0 = clamp(vec3(0.04), base_color.rgb, vec3(msra.r));
        vec3 view_direction = normalize(global.camera_location - position.xyz);
        vec3 light_direction = light_location.rgb - position.rgb;
        float light_distance = length(light_direction);
        light_direction      = normalize(light_direction);

        vec3 view_light_direction = normalize(view_direction + light_direction);

        float attenuation = calc_attenuation(light_radius, light_distance);
        vec3 radiance     = light_color.rgb * light_intensivity * attenuation;

        float ggx      = distribution_ggx(normal.xyz, view_light_direction, msra.b);
        float geometry = geometry_smith(normal.xyz, view_light_direction, light_direction, msra.b);
        vec3 fresnel   = fresnel_schlick(max(dot(view_light_direction, view_direction), 0.0), f0);
        vec3 specular =
                (ggx * geometry * fresnel) /
                (4.0 * max(dot(normal.xyz, view_direction), 0.0) * max(dot(normal.xyz, light_direction), 0.0) + 0.0001);

        vec3 k_s = fresnel;
        vec3 k_d = vec3(1.0, 1.0, 1.0) - k_s;
        k_d *= 1.0 - msra.r;

        float normal_dot_l = max(dot(normal.xyz, light_direction), 0.0);
        vec3 reflectance = (k_d * base_color.xyz / M_PI + specular) * radiance * normal_dot_l;

        light_out_color = reflectance;
    }

    light_out_alpha = 1.0;
}
