import "trinex/quad.slang";
import "trinex/trinex.slang";
import "trinex/math.slang";

struct Args
{
	float2 offset;
	float2 size;
	uint swizzle;
}

uniform Sampler2D scene;

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID, out float2 uv: TEXCOORD0) : SV_Position
{
	uv = args.offset + args.size * FullScreenQuad::uv_by_index(index);
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float2 uv: TEXCOORD0) : SV_Target
{
	return Math::swizzle(scene.SampleLevel(uv, 0.f), args.swizzle);
}
