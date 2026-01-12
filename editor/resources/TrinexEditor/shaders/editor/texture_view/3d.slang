uniform Sampler3D texture;
uniform float depth;
uniform float mip;

export float4 texture_view(float2 uv)
{
	return texture.SampleLevel(float3(uv, depth), mip);
}