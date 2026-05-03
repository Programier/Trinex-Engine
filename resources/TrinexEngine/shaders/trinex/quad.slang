namespace FullScreenQuad
{
	static const float2 quad_uvs[] = {
    	float2(0.0, 0.0),
    	float2(0.0, 1.0),
    	float2(1.0, 0.0),
		float2(0.0, 1.0),
    	float2(1.0, 1.0),
    	float2(1.0, 0.0)
	};

	inline float4 vertex_from_uv(float2 value)
	{
		return float4(value * 2.0 - 1.0, 0.0, 1.0);
	}

	inline float4 vertex(uint index)
	{
		return vertex_from_uv(uv(index));
	}

	inline float4 vertex(uint index, float2 offset, float2 size)
	{
		return vertex_from_uv(uv(index, offset, size));
	}

	inline float2 uv(uint index)
	{
		return quad_uvs[index];
	}

	inline float2 uv(uint index, float2 offset, float2 size)
	{
		return offset + size * uv(index);
	}
}