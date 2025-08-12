import "trinex/trinex.slang";
import "trinex/quad.slang";
import "trinex/scene_view.slang";

struct Args
{
	float2 noise_scale;
	float intensity;
	float bias;
	float power;
	float radius;
	float fade_out_distance;
	float fade_out_radius;
	uint samples;
};

uniform DescriptorHandle<Texture2D> scene_depth;
uniform DescriptorHandle<Texture2D> scene_normal;
uniform DescriptorHandle<Texture2D> noise_texture;
uniform DescriptorHandle<SamplerState> sampler;

[parameter_flags(meta::UniformBuffer)]
uniform Args args;

uniform StructuredBuffer<float3> samples;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

float3 to_view_space(float3 vector)
{
	float4 result = scene_view.view * float4(vector, 1.f);
	return result.xyz / result.w;
}

float3 rotate_normal(float3 normal)
{
	return normalize((float3x3)scene_view.view * normal);
}

[ForceInline]
float2 make_ndc(float3 position)
{
	float4 result = scene_view.projection * float4(position, 1.0);
	return result.xy / result.w;
}

[ForceInline]
bool is_screen_space(float2 ndc)
{
	ndc = abs(ndc);
	return max(ndc.x, ndc.y) <= 1.f;
}

[shader("fragment")]
float4 fragment_main(in float4 position: SV_Position) : SV_Target
{
	float2 screen_uv   = scene_view.screen_uv(position.xy);
	float2 viewport_uv = scene_view.viewport_uv(position.xy);

	float3 noise  = noise_texture.Sample(sampler, viewport_uv * args.noise_scale).xyz * 2.0 - 1.f;
	float3 normal = rotate_normal(scene_normal.Sample(sampler, screen_uv).xyz);
	float3 position = to_view_space(scene_view.reconstruct_position(position.xy, scene_depth.Sample(sampler, screen_uv).r));
	float1 depth    = position.z;

	float occlusion = 0.0;

	for (int i = 0; i < args.samples; ++i)
	{
		float3 sample_offset = reflect(samples[i], noise);
		float2 sample_ndc    = make_ndc(position + (sample_offset * args.radius * sign(dot(sample_offset, normal))));

		if (!is_screen_space(sample_ndc))
			continue;

		float2 sample_uv = sample_ndc.xy * 0.5 + 0.5;

		float3 sample_position = to_view_space(scene_view.reconstruct_position_from_ndc(sample_ndc, scene_depth.Sample(sampler, sample_uv).r));
		float1 sample_depth = sample_position.z;

		float range_check = smoothstep(0.0, 1.0, args.radius / abs(depth - sample_depth));
		occlusion += (sample_depth >= depth + args.bias ? 1.0 : 0.0) * range_check;
	}

	occlusion = 1.f - (occlusion / float(args.samples));

	occlusion = pow(saturate(occlusion * args.intensity), args.power);
	float fade = smoothstep(args.fade_out_distance, args.fade_out_distance - args.fade_out_radius, length(position));
	occlusion  = lerp(1.0, occlusion, fade);
	return float4(0, 0, 0, occlusion);
}