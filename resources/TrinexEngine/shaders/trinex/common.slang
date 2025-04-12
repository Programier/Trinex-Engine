#ifndef TRINEX_COMMON_SLANG
#define TRINEX_COMMON_SLANG

struct GlobalParameters
{
	struct Viewport {
		float2 pos;
		float2 size;
		float min_depth;
		float max_depth;
	};

	struct Camera {
		enum Projection : int
		{
			Perspective  = 0,
			Orthographic = 1,
		};

		float3 location;
		float3 forward;
		float3 right;
		float3 up;

		float fov;
		float ortho_width;
		float ortho_height;
		float near_clip_plane;
		float far_clip_plane;
		float aspect_ratio;
		Projection projection_mode;
	};

	float4x4 projection;
	float4x4 view;
	float4x4 projview;
	float4x4 inv_projview;

	Viewport viewport;
	Camera camera;

	float2 render_target_size;
	float gamma;
	float time;
	float delta_time;

	float2 normalize_screen_space_uv(float2 uv)
    {
        float2 uv0 = viewport.size / render_target_size;
		float2 uv1 = (viewport.size + viewport.pos) / render_target_size;
		return (uv * (uv1 - uv0)) + uv0;
    }
};

struct GBufferFragmentOutput
{
    [vk::location(0)] float4 base_color   : SV_TARGET0;
    [vk::location(1)] float4 normal       : SV_TARGET1;
    [vk::location(2)] float4 emissive     : SV_TARGET2;
    [vk::location(3)] float4 msra         : SV_TARGET3;
};
#endif
