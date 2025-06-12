struct SceneView
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
	Camera camera;

	float2 render_target_size;
	float gamma;
	float time;
	float delta_time;

	float2 normalize_screen_space_uv(float2 uv)
    {
        float2 uv0 = viewport.pos / render_target_size;
		float2 uv1 = (viewport.pos + viewport.size) / render_target_size;
		return (uv * (uv1 - uv0)) + uv0;
    }
};

uniform ConstantBuffer<SceneView> scene_view;