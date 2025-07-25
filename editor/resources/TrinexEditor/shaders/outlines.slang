import "trinex/quad.slang";
import "trinex/random.slang";
import "trinex/math.slang";

struct Arguments
{
    Texture2D scene_color;
    Texture2D scene_depth;
    Texture2D outline_depth;
    SamplerState sampler;

    float3 outline_color;
    float near;
    float far;
    float2 sample_offset;
};

uniform Arguments args;


[ForceInline] float min4(float v0, float v1, float v2, float v3) { return min(min(v0, v1), min(v2, v3)); }

[ForceInline] float random_float(float2 p)
{
    float3 p3 = fract(p.xyx * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

[shader("vertex")]
float4 vertex_main(out float2 uv: TEXCOORD0, in uint vertex: SV_VertexID) : SV_Position
{
    uv = FullScreenQuad::uv_by_index(vertex);
    return FullScreenQuad::vertex_by_index(vertex);
}

[shader("fragment")]
float4 fragment_main(in float2 uv : TEXCOORD0) : SV_Target
{
	float outline_min_depth = min4(
        args.outline_depth.SampleLevel(args.sampler, uv + float2(args.sample_offset.x, 0.f), 0.f).r,
        args.outline_depth.SampleLevel(args.sampler, uv + float2(-args.sample_offset.x, 0.f), 0.f).r,
        args.outline_depth.SampleLevel(args.sampler, uv + float2(0.f, args.sample_offset.y), 0.f).r,
        args.outline_depth.SampleLevel(args.sampler, uv + float2(0.f, -args.sample_offset.y), 0.f).r
    );

    float3 outline_color = args.outline_color;
    float3 scene_color = args.scene_color.SampleLevel(args.sampler, uv, 0.f).rgb;
    float scene_depth = args.scene_depth.SampleLevel(args.sampler, uv, 0.f).r;
    float outline_depth = args.outline_depth.SampleLevel(args.sampler, uv, 0.f).r;

    float intensity = 0;

    [branch]
    if (outline_min_depth != 1.f)
    {
        intensity = 1;

        [branch]
        if (scene_depth < outline_min_depth)
        {
            outline_color *= 0.5f;
        }
    }

    float scene_depth_linear = Math::linearize_depth(scene_depth, args.near, args.far);
    float outline_depth_linear = Math::linearize_depth(outline_depth, args.near, args.far);

    if (abs(scene_depth_linear - outline_depth_linear) < 0.001f && scene_depth < 1.f)
    {
        return float4(scene_color, 1.0f);
    }
    else if (outline_depth >= 1)
    {
        return float4(lerp(scene_color, args.outline_color, intensity), 1.f);
    }
    else
    {
        float mask = Random::pseudo_random(uv);
        return float4(scene_color * lerp(0.6, 0.9f, mask), 1.f);
    }
}
