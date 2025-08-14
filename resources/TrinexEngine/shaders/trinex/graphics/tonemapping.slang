import "trinex/quad.slang";
import "trinex/scene_view.slang";
import "trinex/trinex.slang";

uniform Sampler2D hdr_scene;

[shader("vertex")]
float4 vertex_main(in uint index: SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[shader("fragment")]
float4 fragment_main(in float4 position : SV_Position) : SV_Target
{
	float2 uv          = scene_view.screen_uv(position.xy);
	float4 scene_color = hdr_scene.SampleLevel(uv, 0);
	scene_color.rgb    = ACES(scene_color.rgb);

	// Gamma
	scene_color.rgb = pow(scene_color.rgb, float3(1.0 / 2.2));
	return scene_color;
}

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// MIT License

static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
	{ 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

[ForceInline]
static float3 ACES(float3 linear)
{
	float3 color = linear * ACESInputMat;

	// Apply RRT and ODT
	float3 a = color * (color + 0.0245786f) - 0.000090537f;
	float3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
	color    = a / b;

	color = color * ACESOutputMat;
	return saturate(color);
}