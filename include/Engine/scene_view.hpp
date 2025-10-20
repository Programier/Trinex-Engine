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
		RHIScissor m_scissor;
		ShowFlags m_show_flags;

	public:
		SceneView(ShowFlags show_flags = ShowFlags::DefaultFlags);
		SceneView(const CameraView& view, const Vector2u& view_size, ShowFlags show_flags = ShowFlags::DefaultFlags);
		SceneView(const CameraView& view, const RHIViewport& viewport, const RHIScissor& scissor,
		          ShowFlags show_flags = ShowFlags::DefaultFlags);
		copy_constructors_hpp(SceneView);

	public:
		SceneView& camera_view(const CameraView& view);
		SceneView& viewport(const RHIViewport& viewport);
		SceneView& scissor(const RHIScissor& scissor);
		SceneView& transform(const Matrix4f& transform);

		SceneView& show_flags(ShowFlags flags);
		Vector3f screen_to_ray_direction(const Vector2f& screen_point) const;
		Vector3f uv_to_ray_direction(const Vector2f& uv) const;
		Vector3f world_to_screen(const Vector3f& world) const;
		Vector3f screen_to_world(const Vector3f& screen) const;

		FORCE_INLINE const RHIViewport& viewport() const { return m_viewport; }
		FORCE_INLINE const RHIScissor& scissor() const { return m_scissor; }
		FORCE_INLINE const Matrix4f& view_matrix() const { return m_view; }
		FORCE_INLINE const Matrix4f& projection() const { return m_projection; }
		FORCE_INLINE const Matrix4f& projview() const { return m_projview; }
		FORCE_INLINE const Matrix4f& inv_projview() const { return m_inv_projview; }
		FORCE_INLINE const CameraView& camera_view() const { return m_camera_view; }
		FORCE_INLINE const Vector2u& view_size() const { return m_viewport.size; }
		FORCE_INLINE ShowFlags show_flags() const { return m_show_flags; }
	};
}// namespace Engine
