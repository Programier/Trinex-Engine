#ifndef COMMON_SLANG
#define COMMON_SLANG

struct GlobalParameters
{
    float4x4 projection;
    float4x4 view;
    float4x4 projview;
    float4x4 inv_projview;

    float4 viewport;
    float4 scissor;

    float3 camera_location;
    float3 camera_forward;
    float3 camera_right;
    float3 camera_up;

    float2 size;
    float2 depth_range;

    float gamma;
    float time;
    float delta_time;

    float fov;
    float ortho_width;
    float ortho_height;
    float near_clip_plane;
    float far_clip_plane;
    float aspect_ratio;
    int camera_projection_mode;
};

[vk::binding(0, 0)]
ConstantBuffer<GlobalParameters> globals; 

struct GBufferFragmentOutput
{
    float4 base_color   : SV_TARGET0;
    float4 position     : SV_TARGET1;
    float4 normal       : SV_TARGET2;
    float4 emissive     : SV_TARGET3;
    float4 msra         : SV_TARGET4;
};

struct LightData
{
    float4      light_color     : COLOR0;
    float       intensivity     : BLENDWEIGHT0;
    float3      position        : POSITION1;
    float       radius          : BLENDWEIGHT1;
};

/////////////////////////// MATH ///////////////////////////

float3x3 inverse(float3x3 m)
{
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    float inv_det = 1.0 / det;

    float3x3 inverse_matrix;

    inverse_matrix[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * inv_det;
    inverse_matrix[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * inv_det;
    inverse_matrix[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * inv_det;

    inverse_matrix[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * inv_det;
    inverse_matrix[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * inv_det;
    inverse_matrix[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * inv_det;

    inverse_matrix[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * inv_det;
    inverse_matrix[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * inv_det;
    inverse_matrix[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * inv_det;

    return inverse_matrix;
}

float4x4 inverse(float4x4 m) 
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float inv_det = 1.0 / det;

    float4x4 inverse_matrix;

    inverse_matrix[0][0] = t11 * inv_det;
    inverse_matrix[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * inv_det;
    inverse_matrix[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * inv_det;
    inverse_matrix[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * inv_det;

    inverse_matrix[1][0] = t12 * inv_det;
    inverse_matrix[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * inv_det;
    inverse_matrix[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * inv_det;
    inverse_matrix[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * inv_det;

    inverse_matrix[2][0] = t13 * inv_det;
    inverse_matrix[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * inv_det;
    inverse_matrix[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * inv_det;
    inverse_matrix[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * inv_det;

    inverse_matrix[3][0] = t14 * inv_det;
    inverse_matrix[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * inv_det;
    inverse_matrix[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * inv_det;
    inverse_matrix[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * inv_det;

    return inverse_matrix;
}

float3 rotate_normal(in float3 normal, in float4x4 model)
{
    return normalize(mul(transpose(inverse((float3x3)model)), normal));
}

#endif