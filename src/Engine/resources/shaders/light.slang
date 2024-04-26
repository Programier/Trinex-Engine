#include "common.slang"

#define M_PI 3.141592

uniform Sampler2D base_color_texture;  
uniform Sampler2D position_texture;
uniform Sampler2D normal_texture;
uniform Sampler2D emissive_texture;
uniform Sampler2D msra_texture;

struct LightData
{
    float4      color;
    float3      ambient_color;
    float       intensivity;

#if POINT_LIGHT
    float3      location;
    float       radius;
#elif SPOT_LIGHT
    float3      location;
    float3      direction;
    float       radius;
    float       height;
    float       cutoff;
#else
    float3      direction;
#endif

};

uniform LightData light_data;

float calc_attenuation(in float dist)
{
#if POINT_LIGHT
    return pow(saturate(1.0 - pow(dist / light_data.radius, 4.0)), 2.0) / (pow(dist, 2.0) + 1.0);
#elif SPOT_LIGHT
    return pow(saturate(1.0 - pow(dist / light_data.height, 4.0)), 2.0) / (pow(dist, 2.0) + 1.0);
#else
    return 1.0;
#endif
}

float distribution_ggx(in float3 normal, in float3 view_light_direction, float roughness)
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

float geometry_smith(in float3 normal, in float3 view_light_direction, in float3 light_direction, float roughness)
{
    float normal_dot_v = max(dot(normal, view_light_direction), 0.0);
    float normal_dot_l = max(dot(normal, light_direction), 0.0);
    return geometry_schlick_ggx(normal_dot_l, roughness) * geometry_schlick_ggx(normal_dot_v, roughness);
}

float3 fresnel_schlick(in float cos_theta, in float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

[shader("vertex")]
float4 vs_main(in float3 position : POSITION0, out float2 uv : TEXCOORD0) : SV_Position
{
    uv = (float2(1, 1) + position.xy) / float2(2, 2);
    return float4(position, 1.0);
}

[shader("fragment")]
float4 fs_main(in float2 uv : TEXCOORD0) : SV_Target
{
    float4 base_color =  base_color_texture.Sample(uv);
    float4 position = position_texture.Sample(uv);
    float4 normal = normal_texture.Sample(uv);
    float4 emissive = emissive_texture.Sample(uv); 
    float4 msra = msra_texture.Sample(uv);


    float3 light_out_color;
    if (length(normal.rgb) < 0.1)
    {
        light_out_color = light_data.ambient_color * base_color.rgb * float3(msra.a); // Only ambient lighting
    }
    else
    {
        float3 view_direction = normalize(globals.camera_location - position.xyz);
        float3 surface_normal = normalize(normal.xyz) * normal.w;
        float3 f0 = clamp(float3(0.04), base_color.rgb, float3(msra.r));

#if POINT_LIGHT | SPOT_LIGHT
        float3 light_direction = light_data.location.rgb - position.rgb;
        float attenuation = calc_attenuation(length(light_direction));
        light_direction   = normalize(light_direction);
#else
        float3 light_direction = light_data.direction;
        float attenuation = 1.0;
#endif

        float3 view_light_direction = normalize(view_direction + light_direction);
        float3 radiance     = light_data.color.rgb * light_data.intensivity * attenuation;

        float ggx      = distribution_ggx(surface_normal, view_light_direction, msra.b);
        float geometry = geometry_smith(surface_normal, view_light_direction, light_direction, msra.b);
        float3 fresnel   = fresnel_schlick(max(dot(view_light_direction, view_direction), 0.0), f0);
        float3 specular =
                (ggx * geometry * fresnel) /
                (4.0 * max(dot(surface_normal, view_direction), 0.0) * max(dot(surface_normal, light_direction), 0.0) + 0.0001);

        float3 k_s = fresnel;
        float3 k_d = float3(1.0, 1.0, 1.0) - k_s;
        k_d *= 1.0 - msra.r;

        float normal_dot_l = max(dot(surface_normal, light_direction), 0.0);
        float3 reflectance = (k_d * base_color.xyz / M_PI + specular) * radiance * normal_dot_l;

        float3 ambient = light_data.ambient_color * base_color.rgb * float3(msra.a);
        light_out_color = ambient + reflectance;
    }


    return float4(light_out_color.rgb, 1);
}