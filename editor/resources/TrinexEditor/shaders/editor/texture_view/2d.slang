uniform Sampler2D texture;
uniform float mip;

export float4 texture_view(float2 uv)
{
	return texture.SampleLevel(uv, mip);
}