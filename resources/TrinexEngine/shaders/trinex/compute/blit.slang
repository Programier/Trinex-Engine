#include "trinex/attributes.slang"

struct Arguments
{
	int4 src_rect;
	int4 dst_rect;
	float4 blend;
	uint4 swizzle;

#if TRINEX_BLIT_GAMMA
	float  gamma;
#endif
	uint level;
};

uniform Texture2D<float4> src;
uniform RWTexture2D<float4> dst;

[parameter_type("MemoryBlock")]
uniform Arguments args;

[ForceInline]
static float4 calculate_swizzle(in float4 color)
{
	return float4(color[args.swizzle.x], color[args.swizzle.y], color[args.swizzle.z], color[args.swizzle.w]);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void cs_main(uint3 thread_id : SV_DispatchThreadID)
{
	int2 dst_size = abs(args.dst_rect.zw);

	if (thread_id.x > dst_size.x || thread_id.y > dst_size.y)
        return;

	float2 uv_lerp_factor = float2(thread_id.xy) / float2(dst_size);
	int2 src_coord = args.src_rect.xy + int2(float2(args.src_rect.zw) * uv_lerp_factor);
    int2 dst_coord = args.dst_rect.xy + thread_id.xy * sign(args.dst_rect.zw);
    
    float4 src_color = calculate_swizzle(src[src_coord]);
    float4 dst_color = dst[dst_coord];

#if TRINEX_BLIT_GAMMA
	dst[dst_coord] = pow(lerp(dst_color, src_color, args.blend), args.gamma);

#else
    dst[dst_coord] = lerp(dst_color, src_color, args.blend);
#endif
}