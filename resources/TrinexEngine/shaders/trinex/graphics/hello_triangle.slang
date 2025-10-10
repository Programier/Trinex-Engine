struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VertexOutput vertex_main(in uint index : SV_VertexID)
{
	static float2 positions[3] = {
		float2(-0.5, 0.5),
		float2(0.0, -0.5),
		float2(0.5, 0.5),
	};

	static float3 colors[3] = {
		float3(1.0, 0.0, 0.0),
		float3(0.0, 1.0, 0.0),
		float3(0.0, 0.0, 1.0),
	};

	float3 color  = colors[index];
	float2 pos = positions[index];

	VertexOutput output;
	output.color = color;
	output.position = float4(pos, 0.f, 1.f);
	return output;
}

[shader("fragment")]
float4 fragment_main(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0f);
}