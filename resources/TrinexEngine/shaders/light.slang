#include "trinex/quad.slang"
#include "common.slang"
#include "math.slang"
#include "platform.slang"

uniform Sampler2D base_color_texture;  
uniform Sampler2D normal_texture;
uniform Sampler2D emissive_texture;
uniform Sampler2D msra_texture;
uniform Sampler2D depth_texture;

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

float distribution_ggx(in float3 normal, in float3 halfway_direction, float roughness)
{
    float roughness4    = square(square(roughness));
    float n_dot_h2      = square(max(dot(normal, halfway_direction), 0.0));
    float denom         = n_dot_h2 * (roughness4 - 1.0) + 1.0;
    return roughness4 / (M_PI * (denom * denom));
}

float geometry_schlick_ggx(float normal_dot_v, float roughness)
{
    float r = (roughness + 1.0);
    float coefficient = (r*r) / 8.0;
    return normal_dot_v / (normal_dot_v * (1.0 - coefficient) + coefficient);
}

float geometry_smith(in float3 normal, in float3 halfway_direction, in float3 light_direction, float roughness)
{
    float normal_dot_v = max(dot(normal, halfway_direction), 0.0);
    float normal_dot_l = max(dot(normal, light_direction), 0.0);
    return geometry_schlick_ggx(normal_dot_l, roughness) * geometry_schlick_ggx(normal_dot_v, roughness);
}

float3 fresnel_schlick(in float cos_theta, in float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

float3 normalize_normal(float3 normal, in float3 view)
{
    normal = normalize(normal);
    
    if(dot(normal, view) < 0.0)
        normal = -normal;

    return normal;
}

float fresnel_zero_reflectance(in float specular)
{
    float IOR = lerp(1.3, 1.7, specular);
    return square((IOR - 1.0) / (IOR + 1.0));
}

float3 reconstruct_position(in float2 ndc, in float2 uv) 
{
    float depth = depth_texture.Sample(uv).r;
    float4 clip_space_pos = float4(ndc, lerp(Platform::ndc_depth_range.x, Platform::ndc_depth_range.y, depth), 1.0);
    float4 view_space_pos = mul(globals.inv_projview, clip_space_pos);
    return view_space_pos.xyz /= view_space_pos.w;
}

[shader("vertex")]
float4 vs_main(in uint vertex : SV_VertexID, out float2 uv : TEXCOORD0, out float2 ndc : TEXCOORD1) : SV_Position
{
    float2 min_v = globals.viewport.xy / globals.size;
    float2 max_v = (globals.viewport.xy + globals.viewport.zw) / globals.size;
    uv = Platform::validate_uv((FullScreenQuad::uv_by_index(vertex) * (max_v - min_v)) + min_v);
    
    float4 vertex = FullScreenQuad::vertex_by_index(vertex);
    ndc = vertex.xy;
    return vertex;
}

[shader("fragment")]
float4 fs_main(in float2 uv : TEXCOORD0, in float2 ndc : TEXCOORD1) : SV_Target
{
    float4 base_color =  base_color_texture.Sample(uv);
    float3 position = reconstruct_position(ndc, uv);
    float3 normal = normal_texture.Sample(uv).xyz;
    float4 msra = msra_texture.Sample(uv);

    if (length(normal) < 0.1)
    {
        return float4(0.0, 0.0, 0.0, 1.0);
    }
    
    float3 view_direction = normalize(globals.camera_location - position); // Using view direction from fragment location to camera location
    normal =  normalize_normal(normal, view_direction);

    float3 f0 = lerp(float3(fresnel_zero_reflectance(msra.g)), base_color.rgb, float3(msra.r));

#if POINT_LIGHT || SPOT_LIGHT
    float3 light_direction = light_data.location.rgb - position;           // Using light direction from fragment location to light location
    float attenuation = calc_attenuation(light_direction);
    light_direction   = normalize(light_direction);
#else
    float3 light_direction = -light_data.direction;
    float attenuation = 1.0;
#endif

    float3 halfway_direction = normalize(view_direction + light_direction);
    float3 radiance     = light_data.color.rgb * light_data.intensivity * attenuation;

    float ggx      = distribution_ggx(normal, halfway_direction, msra.b);
    float geometry = geometry_smith(normal, halfway_direction, light_direction, msra.b);
    float3 fresnel = fresnel_schlick(max(dot(halfway_direction, view_direction), 0.0), f0);
    float3 specular = (ggx * geometry * fresnel) / (4.0 * max(dot(normal, view_direction), 0.0) * max(dot(normal, light_direction), 0.0) + 0.001);

    float3 k_s = fresnel;
    float3 k_d = float3(1.0, 1.0, 1.0) - k_s;
    k_d *= 1.0 - msra.r;

    float normal_dot_l = max(dot(normal, light_direction), 0.0);
    float3 reflectance = (k_d * base_color.xyz / M_PI + specular) * radiance * normal_dot_l;
    return float4(reflectance, 1.0);
}
