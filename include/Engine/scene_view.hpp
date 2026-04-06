#pragma once
#include <Engine/camera_view.hpp>
#include <Engine/enums.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class SceneViewState;
	class Scene;
	class Renderer;

	class ENGINE_EXPORT SceneView
	{
	private:
		CameraView m_camera_view;
		CameraView m_prev_camera_view;
		Vector2u m_view_size;
		RHIViewport m_viewport;
		RHIScissor m_scissor;
		ShowFlags m_show_flags;
		Scene* m_scene;
		SceneViewState* m_state = nullptr;

	public:
		SceneView(Scene* scene = nullptr, const CameraView& camera = {}, Vector2u size = {0, 0},
		          ShowFlags flags = ShowFlags::DefaultFlags);
		trinex_default_copyable(SceneView);
		trinex_default_moveable(SceneView);

		const SceneView& flush(Renderer* renderer) const;

		Vector3f screen_to_ray_direction(const Vector2f& screen_point) const;
		Vector3f uv_to_ray_direction(const Vector2f& uv) const;
		Vector3f world_to_screen(const Vector3f& world) const;
		Vector3f screen_to_world(const Vector3f& screen) const;

		inline SceneView& camera_view(const CameraView& view) { trinex_this_return(m_camera_view = view); }
		inline SceneView& prev_camera_view(const CameraView& view) { trinex_this_return(m_prev_camera_view = view); }
		inline SceneView& view_size(Vector2u size) { trinex_this_return(m_view_size = size;); }
		inline SceneView& viewport(const RHIViewport& viewport) { trinex_this_return(m_viewport = viewport;); }
		inline SceneView& scissor(const RHIScissor& scissor) { trinex_this_return(m_scissor = scissor;); }
		inline SceneView& show_flags(ShowFlags flags) { trinex_this_return(m_show_flags = flags;); }
		inline SceneView& scene(Scene* scene) { trinex_this_return(m_scene = scene;); }
		inline SceneView& state(SceneViewState* state) { trinex_this_return(m_state = state;); }

		inline const RHIViewport& viewport() const { return m_viewport; }
		inline const RHIScissor& scissor() const { return m_scissor; }
		inline const CameraView& camera_view() const { return m_camera_view; }
		inline const CameraView& prev_camera_view() const { return m_prev_camera_view; }
		inline const Vector2u& view_size() const { return m_view_size; }
		inline ShowFlags show_flags() const { return m_show_flags; }
		inline Scene* scene() const { return m_scene; }
		inline SceneViewState* state() const { return m_state; }
	};
}// namespace Trinex
