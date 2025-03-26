#include "trinex/attributes.slang"

struct Arguments
{
	float4 src_rect;
	float4 dst_rect;
	float4 src_blend;
	float4 dst_blend;
	uint level;
};

uniform Sampler2D<float4> src;
uniform RWTexture2D<float4> dst;

[parameter_type("MemoryBlock")]
uniform Arguments args;

static float2 src_dimention()
{
	int x, y;   
    src.GetDimensions(x, y);
    return float2(x, y);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void cs_main(uint3 thread_id : SV_DispatchThreadID)
{
	float2 dst_size = abs(args.dst_rect.zw);

	if (thread_id.x > uint(dst_size.x) || thread_id.y > uint(dst_size.y))
        return;

	float2 uv_lerp_factor = float2(float(thread_id.x) / dst_size.x, float(thread_id.y) / dst_size.y);
	float2 uv = lerp(args.src_rect.xy, args.src_rect.xy + args.src_rect.zw, uv_lerp_factor) / src_dimention();
    int2 dst_coord = int2(args.dst_rect.xy) + int2(thread_id.xy) * sign(args.dst_rect.zw);
    
    float4 src_color = src.SampleLevel(uv, args.level);
    float4 dst_color = dst[dst_coord];
    dst[dst_coord] = src_color * args.src_blend + dst_color * args.dst_blend;
}