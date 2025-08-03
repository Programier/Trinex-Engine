#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/matrix.hpp>

namespace Engine
{
	struct GlobalShaderParameters {
		struct Viewport {
			alignas(8) Vector2f pos;
			alignas(8) Vector2f size;
			alignas(8) Vector2f inv_size;

			alignas(4) float min_depth;
			alignas(4) float max_depth;
		};

		struct RenderTarget {
			alignas(8) Vector2f size;
			alignas(8) Vector2f inv_size;
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
		alignas(16) Matrix4f inv_projection;
		alignas(16) Matrix4f inv_view;
		alignas(16) Matrix4f inv_projview;

		Viewport viewport;
		RenderTarget render_target;
		Camera camera;

		alignas(4) float time;
		alignas(4) float delta_time;

		ENGINE_EXPORT GlobalShaderParameters& update(const class SceneView* scene_view, Vector2u target_size);
	};
}// namespace Engine
