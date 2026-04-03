static const float TAA_ALPHA = 0.05f;

uniform RWTexture2D<float4> scene_color;
uniform RWTexture2D<float4> prev_scene_color;

float3 tone_map(float3 color) 
{
    return color / (1.0f + color);
}

float3 inverse_tone_map(float3 color) 
{
    return color / max(1.0f - color, 1e-4f);
}

#pragma once

[shader("compute")]
[numthreads(8, 8, 1)]
void compute_main(uint3 thread : SV_DispatchThreadID)
{
	int2 dimension;
	scene_color.GetDimensions(dimension.x, dimension.y);
	int2 pixel_coords = thread.xy;

	if (pixel_coords.x >= dimension.x || pixel_coords.y >= dimension.y)
		return;

	float3 current_color_hdr = scene_color[pixel_coords].rgb;
	float3 history_color_hdr = prev_scene_color[pixel_coords].rgb;

    float3 current_color_ldr = tone_map(current_color_hdr);
    float3 history_color_ldr = tone_map(history_color_hdr);

	float3 final_color_ldr = history_color_ldr + (current_color_ldr - history_color_ldr) * TAA_ALPHA;

    float3 final_color_hdr = inverse_tone_map(final_color_ldr);
	scene_color[pixel_coords]      = float4(final_color_hdr, 1.0f);
	prev_scene_color[pixel_coords] = float4(final_color_hdr, 1.0f);
}