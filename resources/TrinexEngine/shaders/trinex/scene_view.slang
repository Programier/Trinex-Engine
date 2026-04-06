import "trinex/trinex.slang";
import "trinex/scene.slang";

struct SceneView
{
	struct Viewport {
		float2 pos;
		float2 size;
		float2 inv_size;
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
		float2 jitter;

		float near;
		float far;

		property float3 right { [ForceInline] get { return inv_view[0].xyz; } }
		property float3 up { [ForceInline] get { return inv_view[1].xyz; } }
		property float3 forward { [ForceInline] get { return inv_view[2].xyz; } }
		property float3 location { [ForceInline] get { return inv_view[3].xyz; } }

		[ForceInline]
		float4 apply_jitter(float4 clip)
		{
			clip.xy += jitter * clip.w;
			return clip;
		}

		[ForceInline] float4 world_to_clip(float3 point) { return projview * float4(point, 1.0f); }
		[ForceInline] float4 view_to_clip(float3 point) { return projection * float4(point, 1.0f); }
		[ForceInline] float4 world_to_view(float3 point) { return view * float4(point, 1.0f); }

		[ForceInline] float4 world_to_clip(float4 point) { return projview * point; }
		[ForceInline] float4 view_to_clip(float4 point) { return projection * point; }
		[ForceInline] float4 world_to_view(float4 point) { return view * point; }

		[ForceInline] float4 world_to_clip_jittered(float3 point) { return apply_jitter(world_to_clip(point)); }
		[ForceInline] float4 view_to_clip_jittered(float3 point) { return apply_jitter(view_to_clip(point)); }

		[ForceInline] float4 world_to_clip_jittered(float4 point) { return apply_jitter(world_to_clip(point)); }
		[ForceInline] float4 view_to_clip_jittered(float4 point) { return apply_jitter(view_to_clip(point)); }

		[ForceInline]
		float3 clip_to_world(float4 clip)
		{
			float4 position = inv_projview * clip;
			return position.xyz / position.w;
		}

		[ForceInline]
		float3 clip_to_view(float4 clip)
		{
			float4 position = inv_projection * clip;
			return position.xyz / position.w;
		}

		[ForceInline] float3 clip_to_world(float3 clip) { return clip_to_world(float4(clip, 1.0f)); }
		[ForceInline] float3 clip_to_world(float2 clip, float depth) { return clip_to_world(float4(clip, depth, 1.0f)); }
		[ForceInline] float3 clip_to_view(float3 clip) { return clip_to_view(float4(clip, 1.0f)); }
		[ForceInline] float3 clip_to_view(float2 clip, float depth) { return clip_to_view(float4(clip, depth, 1.0f)); }
		[ForceInline] float3 view_to_world(float3 point) { return (inv_view * float4(point, 1.0f)).xyz; }
	};

	Viewport viewport;
	RenderTarget render_target;
	Camera camera;
	Camera prev_camera;

	RWStructuredBuffer<Scene>::Handle scene;

	float time;
	float delta_time;

	inline int2 screen_coord(float2 coord) { return int2(viewport.pos + coord); }
	inline float2 screen_uv(float2 coord) { return (viewport.pos + coord) * render_target.inv_size; }
	inline float2 screen_ndc(float2 coord) { return (coord * viewport.inv_size) * 2.f - 1.f; }
	
	inline float2 viewport_uv(float2 coord) { return coord * viewport.inv_size; }
};

uniform ConstantBuffer<SceneView> scene_view;