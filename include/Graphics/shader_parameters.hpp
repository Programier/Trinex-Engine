#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	// Using GLSL std140 alignment rules

	// 400 bytes
	struct GlobalShaderParameters {
		struct Viewport {
			alignas(8) Vector2f pos;
			alignas(8) Vector2f size;
			alignas(4) float min_depth;
			alignas(4) float max_depth;
		};

		struct Camera {
			enum Projection : int
			{
				Perspective  = 0,
				Orthographic = 1,
			};

			alignas(16) Vector3f location;
			alignas(16) Vector3f forward;
			alignas(16) Vector3f right;
			alignas(16) Vector3f up;

			alignas(4) float fov;
			alignas(4) float ortho_width;
			alignas(4) float ortho_height;
			alignas(4) float near;
			alignas(4) float far;
			alignas(4) float aspect_ratio;
			alignas(4) int projection_mode;
		};

		alignas(16) Matrix4f projection;
		alignas(16) Matrix4f view;
		alignas(16) Matrix4f projview;
		alignas(16) Matrix4f inv_projview;

		Viewport viewport;
		Camera camera;

		alignas(8) Vector2f render_target_size;
		alignas(4) float gamma;
		alignas(4) float time;
		alignas(4) float delta_time;

		ENGINE_EXPORT GlobalShaderParameters& update(const class SceneView* scene_view, Size2D render_target_size = {-1.f, -1.f});
	};
}// namespace Engine
