import "trinex/quad.slang";
import "trinex/trinex.slang";
import "trinex/math.slang";

struct Args
{
	float2 src_offset;
	float2 src_size;
	float2 dst_offset;
	float2 dst_size;
	uint swizzle;
}

uniform Sampler2D scene;

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID, out float2 uv: TEXCOORD0) : SV_Position
{
	uv = FullScreenQuad::uv(index, args.src_offset, args.src_size);
	return FullScreenQuad::vertex(index, args.dst_offset, args.dst_size);
}

[shader("fragment")]
float4 fragment_main(in float2 uv: TEXCOORD0) : SV_Target
{
	return Math::swizzle(scene.SampleLevel(uv, 0.f), args.swizzle);
}
