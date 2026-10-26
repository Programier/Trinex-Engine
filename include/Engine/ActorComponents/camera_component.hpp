#pragma once

#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/enums.hpp>

namespace Trinex
{
	struct CameraView;

	class trinex_api CameraComponent : public SceneComponent
	{
		trinex_class(CameraComponent, SceneComponent);

	public:
		trinex_property()
		CameraProjectionMode projection_mode = CameraProjectionMode::Perspective;

		trinex_property()
		float fov = 75.f;

		trinex_property()
		float ortho_width = 20.f;

		trinex_property()
		float ortho_height = 20.f;

		trinex_property()
		float near = 0.1f;

		trinex_property()
		float far = 1000.f;

		bool serialize(Archive& archive) override;
		CameraView camera_view(const Transform& transform, float aspect = 1.f) const;
		CameraView camera_view(float aspect = 1.f) const;
		Matrix4f projection_matrix(float aspect = 1.f);
		Matrix4f view_matrix();
	};
} // namespace Trinex
