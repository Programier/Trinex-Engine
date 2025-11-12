#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/camera_view.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/window.hpp>

namespace Engine
{
	trinex_implement_engine_class(CameraComponent, 0)
	{
		trinex_refl_prop(projection_mode);
		trinex_refl_prop(fov);
		trinex_refl_prop(ortho_width);
		trinex_refl_prop(ortho_height);
		trinex_refl_prop(near);
		trinex_refl_prop(far);
	}

	bool CameraComponent::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		return archive.serialize(projection_mode, fov, ortho_width, ortho_height, near, far);
	}

	CameraView CameraComponent::camera_view(const Transform& transform, float aspect) const
	{
		if (projection_mode == CameraProjectionMode::Perspective)
		{
			return CameraView::static_perspective(transform.location, transform.forward_vector(), transform.up_vector(),
			                                      Math::radians(fov), aspect, near, far);
		}
		else
		{
			return CameraView::static_ortho(transform.location, transform.forward_vector(), transform.up_vector(),
			                                -ortho_width / 2.0f, ortho_width / 2.0f, -ortho_height / 2.0f, ortho_height / 2.0f,
			                                near, far);
		}
	}

	CameraView CameraComponent::camera_view(float aspect) const
	{
		return camera_view(world_transform(), aspect);
	}

	CameraView CameraComponent::previous_camera_view(float aspect) const
	{
		return camera_view(previous_world_transform(), aspect);
	}

	Matrix4f CameraComponent::projection_matrix(float aspect)
	{
		if (projection_mode == CameraProjectionMode::Perspective)
		{
			return Math::perspective(Math::radians(fov), aspect, near, far);
		}
		else
		{
			return Math::ortho(-ortho_width / 2.0f, // Left
			                   ortho_width / 2.0f,  // Right
			                   -ortho_height / 2.0f,// Bottom
			                   ortho_height / 2.0f, // Top
			                   near,                // Near clipping plane
			                   far                  // Far clipping plane
			);
		}
	}

	Matrix4f CameraComponent::view_matrix()
	{
		const Transform& transform = world_transform();
		return Math::look_at(transform.location, transform.location + transform.forward_vector(), transform.up_vector());
	}

	Matrix4f CameraComponent::previous_view_matrix()
	{
		const Transform& transform = previous_world_transform();
		return Math::look_at(transform.location, transform.location + transform.forward_vector(), transform.up_vector());
	}
}// namespace Engine
