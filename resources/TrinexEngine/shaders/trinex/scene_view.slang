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
		float near;
		float far;
		float aspect_ratio;
		Projection projection_mode;
	};

	float4x4 projection;
	float4x4 view;
	float4x4 projview;
	float4x4 inv_projview;

	Viewport viewport;
	RenderTarget render_target;
	Camera camera;

	float time;
	float delta_time;

	inline float2 screen_uv(float2 coord) { return (viewport.pos + coord) * render_target.inv_size; }
	inline float2 screen_ndc(float2 coord) { return (coord * viewport.inv_size) * 2.f - 1.f; }

	inline float3 reconstruct_position(float2 coord, float depth)
	{
		float4 clip_space_pos = float4(screen_ndc(coord), depth, 1.0);
		float4 view_space_pos = mul(scene_view.inv_projview, clip_space_pos);
		return view_space_pos.xyz /= view_space_pos.w;
	}
};

uniform ConstantBuffer<SceneView> scene_view;