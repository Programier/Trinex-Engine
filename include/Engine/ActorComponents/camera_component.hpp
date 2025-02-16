#pragma once

#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/camera_types.hpp>

namespace Engine
{
	class ENGINE_EXPORT CameraComponent : public SceneComponent
	{
		declare_class(CameraComponent, SceneComponent);

	public:
		CameraProjectionMode projection_mode = CameraProjectionMode::Perspective;
		float fov                            = 75.f;
		float ortho_width                    = 1000.f;
		float ortho_height                   = 1000.f;
		float near_clip_plane                = 1.f;
		float far_clip_plane                 = 1000.f;
		float aspect_ratio                   = 1.f;

		bool serialize(Archive& archive) override;
		const CameraComponent& camera_view(CameraView& out) const;
		CameraView camera_view() const;
		Matrix4f projection_matrix();
		Matrix4f view_matrix();
		static Matrix4f view_matrix(const Vector3f& position, const Vector3f& direction, const Vector3f& up_vector);
	};
}// namespace Engine
