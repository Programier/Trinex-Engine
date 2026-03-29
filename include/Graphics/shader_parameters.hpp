#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/matrix.hpp>

namespace Trinex
{
	struct GlobalShaderParameters {
		struct alignas(16) Viewport {
			alignas(8) Vector2f pos;
			alignas(8) Vector2f size;
			alignas(8) Vector2f inv_size;
		};

		struct alignas(16) RenderTarget {
			alignas(8) Vector2f size;
			alignas(8) Vector2f inv_size;
		};

		struct alignas(16) Camera {
			alignas(16) Matrix4f projection;
			alignas(16) Matrix4f view;
			alignas(16) Matrix4f projview;
			alignas(16) Matrix4f inv_projection;
			alignas(16) Matrix4f inv_view;
			alignas(16) Matrix4f inv_projview;
			alignas(8) Vector2f jitter;

			alignas(4) float near;
			alignas(4) float far;
		};

		Viewport viewport;
		RenderTarget render_target;
		Camera camera;
		Camera prev_camera;

		alignas(4) float time;
		alignas(4) float delta_time;

		ENGINE_EXPORT GlobalShaderParameters& update(const class SceneView* scene_view, Vector2u target_size);
	};
}// namespace Trinex
