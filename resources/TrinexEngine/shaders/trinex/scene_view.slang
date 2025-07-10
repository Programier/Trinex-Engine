struct SceneView
{
	struct Viewport {
		float2 pos;
		float2 size;
		float2 target_size;
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

	float time;
	float delta_time;

	float2 screen_uv(float2 uv)
    {
        float2 uv0 = viewport.pos / viewport.target_size;
		float2 uv1 = (viewport.pos + viewport.size) / viewport.target_size;
		return (uv * (uv1 - uv0)) + uv0;
    }
};

uniform ConstantBuffer<SceneView> scene_view;