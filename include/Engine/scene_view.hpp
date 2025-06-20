#pragma once
#include <Engine/camera_types.hpp>
#include <Engine/enums.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class ENGINE_EXPORT SceneView
	{
	private:
		CameraView m_camera_view;
		Matrix4f m_projection;
		Matrix4f m_view;
		Matrix4f m_projview;
		Matrix4f m_inv_projview;
		RHIViewport m_viewport;
		RHIScissors m_scissor;
		ShowFlags m_show_flags;

	public:
		SceneView(ShowFlags show_flags = ShowFlags::DefaultFlags);
		SceneView(const CameraView& view, const Size2D& view_size, ShowFlags show_flags = ShowFlags::DefaultFlags);
		SceneView(const CameraView& view, const RHIViewport& viewport, const RHIScissors& scissor,
		          ShowFlags show_flags = ShowFlags::DefaultFlags);
		copy_constructors_hpp(SceneView);

	public:
		SceneView& camera_view(const CameraView& view);
		SceneView& viewport(const RHIViewport& viewport);
		SceneView& scissor(const RHIScissors& scissor);

		SceneView& show_flags(ShowFlags flags);
		const SceneView& screen_to_world(const Vector2f& screen_point, Vector3f& world_origin, Vector3f& world_direction) const;
		Vector4f world_to_screen(const Vector3f& world_point) const;

		FORCE_INLINE const RHIViewport& viewport() const { return m_viewport; }
		FORCE_INLINE const RHIScissors& scissor() const { return m_scissor; }
		FORCE_INLINE const Matrix4f& view_matrix() const { return m_view; }
		FORCE_INLINE const Matrix4f& projection_matrix() const { return m_projection; }
		FORCE_INLINE const Matrix4f& projview_matrix() const { return m_projview; }
		FORCE_INLINE const Matrix4f& inv_projview_matrix() const { return m_inv_projview; }
		FORCE_INLINE const CameraView& camera_view() const { return m_camera_view; }
		FORCE_INLINE const Vector2i& view_size() const { return m_viewport.size; }
		FORCE_INLINE ShowFlags show_flags() const { return m_show_flags; }
	};
}// namespace Engine
