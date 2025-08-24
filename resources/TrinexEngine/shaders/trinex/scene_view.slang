import "trinex/trinex.slang";

struct SceneView
{
	struct Viewport {
		float2 pos;
		float2 size;
		float2 inv_size;

		float min_depth;
		float max_depth;
	};

	struct RenderTarget
	{
		float2 size;
		float2 inv_size;
	};

	struct Camera {
		float4x4 projection;
		float4x4 view;
		float4x4 projview;
		float4x4 inv_projection;
		float4x4 inv_view;
		float4x4 inv_projview;

		float3 location;
		float near;
		float far;
	};

	Viewport viewport;
	RenderTarget render_target;
	Camera camera;

	float time;
	float delta_time;

	inline float2 screen_uv(float2 coord) { return (viewport.pos + coord) * render_target.inv_size; }
	inline float2 screen_uv_from_ndc(float2 ndc) { return screen_uv((ndc * 0.5f + 0.5f) * viewport.size); }
	inline float2 viewport_uv(float2 coord) { return coord * viewport.inv_size; }
	inline float2 viewport_uv_from_ndc(float2 ndc) { return ndc * 0.5f + 0.5f; }
	inline float2 screen_ndc(float2 coord) { return (coord * viewport.inv_size) * 2.f - 1.f; }

	inline float3 reconstruct_position(float2 coord, float depth)
	{
		float4 clip_space_pos = float4(screen_ndc(coord), depth, 1.0);
		float4 view_space_pos = scene_view.camera.inv_projview * clip_space_pos;
		return view_space_pos.xyz /= view_space_pos.w;
	}

	inline float3 reconstruct_position_from_ndc(float2 ndc, float depth)
	{
		float4 clip_space_pos = float4(ndc, depth, 1.0);
		float4 view_space_pos = scene_view.camera.inv_projview * clip_space_pos;
		return view_space_pos.xyz /= view_space_pos.w;
	}
};

uniform ConstantBuffer<SceneView> scene_view;