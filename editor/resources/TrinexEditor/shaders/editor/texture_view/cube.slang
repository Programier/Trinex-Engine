uniform SamplerCube texture;
uniform float mip;
uniform uint face;

static const float3x3 s_face_basis[6] = {
	{ float3(0, 0, -1), float3(0, -1, 0), float3(1, 0, 0) },
    { float3(0, 0, 1), float3(0, -1, 0), float3(-1, 0, 0) },
	{ float3(1, 0, 0), float3(0, 0, 1), float3(0, 1, 0) },
    { float3(1, 0, 0), float3(0, 0, -1), float3(0, -1, 0) },
	{ float3(1, 0, 0), float3(0, -1, 0), float3(0, 0, 1) },
    { float3(-1, 0, 0), float3(0, -1, 0), float3(0, 0, -1) },
};

export float4 texture_view(float2 uv)
{
	float2 p = uv * 2.0f - 1.0f;
	float3 dir = normalize(
        p.x * s_face_basis[face][0] +
        p.y * s_face_basis[face][1] +
              s_face_basis[face][2]
    );

	return texture.SampleLevel(dir, mip);
}