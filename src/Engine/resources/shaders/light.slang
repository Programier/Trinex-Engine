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
    float       intensivity;
#if POINT_LIGHT || SPOT_LIGHT
    float3      location;
    float       radius;
    float       fall_off_exponent;
#if SPOT_LIGHT
    float3      direction;
    float2      spot_angles; // {Cos outer cone, inv cos cone difference. }
#endif
#else
    float3      direction;
#endif

};

uniform LightData light_data;

float square(float x)
{
    return x * x;
}

float calc_attenuation(float3 world_light_vector)
{
#if POINT_LIGHT || SPOT_LIGHT
    world_light_vector /= light_data.radius;
    float len_sqr = dot(world_light_vector, world_light_vector);
    float attenuation = pow(1.0f - saturate(len_sqr), light_data.fall_off_exponent);

#if SPOT_LIGHT
    float cos_outer_cone = light_data.spot_angles.x;
    float inv_cos_cone_difference = light_data.spot_angles.y;
    attenuation *= square(saturate((dot(normalize(world_light_vector), -light_data.direction) - cos_outer_cone) * inv_cos_cone_difference));
#endif

    return attenuation;
#else
    return 1.0;
#endif
}

float distribution_ggx(in float3 normal, in float3 view_light_direction, float roughness)
{
    float roughness2    = roughness * roughness;
    float roughness4    = roughness2 * roughness2;   
    float normal_dot_h2 = max(dot(normal, view_light_direction), 0.0);
    float denom         = normal_dot_h2 * (roughness4 - 1.0) + 1.0;
    return roughness4 / (M_PI * (denom * denom));
}

float geometry_schlick_ggx(float normal_dot_v, float roughness)
{
    float r = (roughness + 1.0);
    float coefficient = (r*r) / 8.0;
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
        light_out_color = float3(0.0f);
    }
    else
    {
        float3 view_direction = normalize(globals.camera_location - position.xyz); // Using view direction from fragment location to camera location
        float3 surface_normal = normalize(normal.xyz) * normal.w;
        float3 f0 = lerp(float3(0.04), base_color.rgb, float3(msra.r));

#if POINT_LIGHT || SPOT_LIGHT
        float3 light_direction = light_data.location.rgb - position.rgb;           // Using light direction from fragment location to light location
        float attenuation = calc_attenuation(light_direction);
        light_direction   = normalize(light_direction);
#else
        float3 light_direction = -light_data.direction;
        float attenuation = 1.0;
#endif
        float3 view_light_direction = normalize(view_direction + light_direction);
        float3 radiance     = light_data.color.rgb * light_data.intensivity * attenuation;

        float ggx      = distribution_ggx(surface_normal, view_light_direction, msra.b);
        float geometry = geometry_smith(surface_normal, view_light_direction, light_direction, msra.b);
        float3 fresnel = fresnel_schlick(max(dot(view_light_direction, view_direction), 0.0), f0);
        float3 specular = msra.g * (ggx * geometry * fresnel) / (4.0 * max(dot(surface_normal, view_direction), 0.0) * max(dot(surface_normal, light_direction), 0.0) + 0.0001);

        float3 k_s = fresnel;
        float3 k_d = float3(1.0, 1.0, 1.0) - k_s;
        k_d *= 1.0 - msra.r;

        float normal_dot_l = max(dot(surface_normal, light_direction), 0.0);
        float3 reflectance = (k_d * base_color.xyz / M_PI + specular) * radiance * normal_dot_l;
        light_out_color = reflectance;
    }

     // HDR tonemapping
    light_out_color = light_out_color / (light_out_color + float3(1.0));

    // gamma correct
    light_out_color = pow(light_out_color, float3(1.0 / 2.2)); 
    return float4(light_out_color.rgb, 1);
}