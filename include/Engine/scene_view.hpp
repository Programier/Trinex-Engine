#pragma once
#include <Engine/camera_view.hpp>
#include <Engine/enums.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	struct FrameHistory;

	class ENGINE_EXPORT SceneView
	{
	private:
		CameraView m_camera_view;
		CameraView m_prev_camera_view;
		Vector2u m_view_size;
		RHIViewport m_viewport;
		RHIScissor m_scissor;
		ShowFlags m_show_flags;

		FrameHistory* m_history = nullptr;

	public:
		SceneView(ShowFlags show_flags = ShowFlags::DefaultFlags);
		SceneView(const CameraView& view, const Vector2u& view_size, ShowFlags show_flags = ShowFlags::DefaultFlags);

		trinex_default_copyable(SceneView);

	public:
		SceneView& camera_view(const CameraView& view);
		SceneView& prev_camera_view(const CameraView& view);
		SceneView& view_size(Vector2u size);
		SceneView& viewport(const RHIViewport& viewport);
		SceneView& scissor(const RHIScissor& scissor);
		SceneView& show_flags(ShowFlags flags);
		SceneView& history(FrameHistory* history);

		Vector3f screen_to_ray_direction(const Vector2f& screen_point) const;
		Vector3f uv_to_ray_direction(const Vector2f& uv) const;
		Vector3f world_to_screen(const Vector3f& world) const;
		Vector3f screen_to_world(const Vector3f& screen) const;

		FORCE_INLINE const RHIViewport& viewport() const { return m_viewport; }
		FORCE_INLINE const RHIScissor& scissor() const { return m_scissor; }
		FORCE_INLINE const CameraView& camera_view() const { return m_camera_view; }
		FORCE_INLINE const CameraView& prev_camera_view() const { return m_prev_camera_view; }
		FORCE_INLINE const Vector2u& view_size() const { return m_view_size; }
		FORCE_INLINE ShowFlags show_flags() const { return m_show_flags; }
		FORCE_INLINE FrameHistory* history() const { return m_history; }
	};
}// namespace Engine
