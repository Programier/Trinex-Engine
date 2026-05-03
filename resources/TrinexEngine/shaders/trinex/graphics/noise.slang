import "trinex/trinex.slang";
import "trinex/quad.slang";
import "trinex/math.slang";

uniform Sampler2D noise;

struct Args
{
	float2 offset;
	float2 size;
	float2 inv_noise_size;
	float opacity;
	float scale;
};

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex(index, args.offset, args.size);
}

inline float2 noise_uv_from_screen(float2 screen, Sampler2D noise_texture, float scale)
{
	return screen * args.inv_noise_size * scale;
}

[shader("fragment")]
float4 fragment_main(float2 screen : SV_Position) : SV_Target
{
	float2 uv   = screen * args.inv_noise_size * args.scale;
	float noise = saturate(Math::grayscale(noise.SampleLevel(uv, 0).xyz)) - 0.5f;
	return float4(float3(noise * args.opacity), 1.f);
}

