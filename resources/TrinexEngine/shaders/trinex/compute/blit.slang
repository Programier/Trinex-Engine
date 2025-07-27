import "trinex/trinex.slang";

struct Arguments
{
	int4 src_rect;
	int4 dst_rect;
	uint4 swizzle;// [0 -> R, 1 -> G, 2 -> B, 3 -> A, 4 -> 0.f, 5 -> 1.f]
	uint level;
};

uniform Texture2D<float4> src;
uniform writeonly RWTexture2D<float4> dst;

[parameter_flags(meta::UniformBuffer)]
uniform Arguments args;

[ForceInline]
static float4 calculate_swizzle(in float4 color)
{
	const float swizzle[6] = {
		color.r,
		color.g,
		color.b,
		color.a,
		0.f,
		1.f
	};

	return float4(swizzle[args.swizzle.r], swizzle[args.swizzle.g], swizzle[args.swizzle.b], swizzle[args.swizzle.a]);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void compute_main(uint3 thread_id : SV_DispatchThreadID)
{
	int2 dst_size = abs(args.dst_rect.zw);

	if (thread_id.x > dst_size.x || thread_id.y > dst_size.y)
        return;

	float2 uv_lerp_factor = float2(thread_id.xy) / float2(dst_size);
	int2 src_coord = args.src_rect.xy + int2(float2(args.src_rect.zw) * uv_lerp_factor);
    int2 dst_coord = args.dst_rect.xy + thread_id.xy * sign(args.dst_rect.zw);
    
    float4 src_color = calculate_swizzle(src[src_coord]);
	dst[dst_coord] = src_color;
}