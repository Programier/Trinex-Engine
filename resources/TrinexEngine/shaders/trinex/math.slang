namespace Math
{
    static const float pi = 3.141592f;

    float3x3 inverse(in float3x3 m)
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

    float4x4 inverse(in float4x4 m) 
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

    float3 rotate_vector(in float3 normal, in float4x4 model)
    {
        return normalize(mul(transpose(inverse((float3x3)model)), normal));
    }

    float square(float x)
    {
        return x * x;
    }

	float3x3 tangent_basis(float3 normal, float3 pos, float2 uv)
	{
		float3 dp1       = ddx(pos);
		float3 dp2       = ddy(pos);
		float2 duv1      = ddx(uv);
		float2 duv2      = ddy(uv);
		float3 dp2perp   = cross(dp2, normal);
		float3 dp1perp   = cross(normal, dp1);
		float3 tangent   = normalize(dp2perp * duv1.x + dp1perp * duv2.x);
		float3 bitangent = normalize(dp2perp * duv1.y + dp1perp * duv2.y);
		return float3x3(tangent, bitangent, normal);
	}

	float3x3 tangent_basis(float3 normal, float3 tangent, float3 bitangent)
	{
		float3x3 result = float3x3(tangent, bitangent, normal);
#if TRINEX_VULKAN_RHI
		return transpose(result);
#else
		return result;
#endif
	}

	float3x3 tangent_basis(float3 normal, float3 tangent)
	{
		return tangent_basis(normal, tangent, cross(normal, tangent));
	}

	float3x3 tangent_basis(float3 normal)
	{
		return tangent_basis(normal, cross(normal, float3(1, 0, 0)));
	}
}
