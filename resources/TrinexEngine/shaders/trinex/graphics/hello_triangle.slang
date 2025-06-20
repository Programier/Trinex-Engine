

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

[shader("vertex")]
VertexOutput vs_main(in uint index : SV_VertexID)
{
	static float2 uvs[6] = {
		float2(0, 0),
		float2(0, 1),
		float2(1, 1),
        float2(0, 0),
        float2(1, 1),
        float2(1, 0),
	};

	float2 uv = uvs[index];

	VertexOutput output;
	output.uv       = uv;
	output.position = float4((uv * 2) - float2(1, 1), 0.f, 1.f);
	return output;
}

[shader("fragment")]
float4 fs_main(VertexOutput input) : SV_Target
{
    return float4(input.uv, 0.f, 1.0f);
}