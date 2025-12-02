import "trinex/trinex.slang";
import "trinex/quad.slang";
import "trinex/math.slang";

uniform Sampler2D source;

struct Arguments
{
	float2 offset;
	float2 inv_size;
	uint swizzle;
};

[parameter_type(meta::type::UniformBuffer)]
uniform Arguments args;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float4 position: SV_Position) : SV_Target
{
	float2 uv    = args.offset + position.xy * args.inv_size;
	float4 color = source.SampleLevel(uv, 0);
	return Math::swizzle(color, args.swizzle);
}