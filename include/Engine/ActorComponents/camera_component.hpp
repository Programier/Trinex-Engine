#pragma once

#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/enums.hpp>

namespace Engine
{
	struct CameraView;

	class ENGINE_EXPORT CameraComponent : public SceneComponent
	{
		trinex_declare_class(CameraComponent, SceneComponent);

	public:
		CameraProjectionMode projection_mode = CameraProjectionMode::Perspective;
		float fov                            = 75.f;
		float ortho_width                    = 20.f;
		float ortho_height                   = 20.f;
		float near                           = 0.1f;
		float far                            = 1000.f;

		bool serialize(Archive& archive) override;
		CameraView camera_view(const Transform& transform, float aspect = 1.f) const;
		CameraView camera_view(float aspect = 1.f) const;
		CameraView prev_camera_view(float aspect = 1.f) const;
		Matrix4f projection_matrix(float aspect = 1.f);
		Matrix4f view_matrix();
		Matrix4f prev_view_matrix();
	};
}// namespace Engine
