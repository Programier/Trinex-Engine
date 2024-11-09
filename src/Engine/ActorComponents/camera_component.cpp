#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/window.hpp>
#include <glm/ext.hpp>

namespace Engine
{
	Matrix4f CameraView::projection_matrix() const
	{
		if (projection_mode == CameraProjectionMode::Perspective)
		{
			return glm::perspective(glm::radians(fov), aspect_ratio, near_clip_plane, far_clip_plane);
		}
		else if (projection_mode == CameraProjectionMode::Orthographic)
		{
			return glm::ortho(-ortho_width / 2.0f, // Left
			                  ortho_width / 2.0f,  // Right
			                  -ortho_height / 2.0f,// Bottom
			                  ortho_height / 2.0f, // Top
			                  near_clip_plane,     // Near clipping plane
			                  far_clip_plane       // Far clipping plane
			);
		}

		return Matrix4f(1.f);
	}

	Matrix4f CameraView::view_matrix() const
	{
		return view_matrix(location, forward_vector, up_vector);
	}

	ENGINE_EXPORT Matrix4f CameraView::view_matrix(const Vector3D& position, const Vector3D& direction, const Vector3D& up_vector)
	{
		return glm::lookAt(position, position + direction, up_vector);
	}

	implement_engine_class_default_init(CameraComponent, 0);

	bool CameraComponent::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		archive & projection_mode;
		archive & fov;
		archive & ortho_width;
		archive & ortho_height;
		archive & near_clip_plane;
		archive & far_clip_plane;
		archive & aspect_ratio;


		return static_cast<bool>(archive);
	}

	Matrix4f CameraComponent::projection_matrix()
	{
		if (projection_mode == CameraProjectionMode::Perspective)
		{
			return glm::perspective(glm::radians(fov), aspect_ratio, near_clip_plane, far_clip_plane);
		}
		else if (projection_mode == CameraProjectionMode::Orthographic)
		{
			return glm::ortho(-ortho_width / 2.0f, // Left
			                  ortho_width / 2.0f,  // Right
			                  -ortho_height / 2.0f,// Bottom
			                  ortho_height / 2.0f, // Top
			                  near_clip_plane,     // Near clipping plane
			                  far_clip_plane       // Far clipping plane
			);
		}

		return Matrix4f(1.0);
	}

	Matrix4f CameraComponent::view_matrix()
	{
		auto& global_transfrom = world_transform();
		return view_matrix(global_transfrom.location(), global_transfrom.forward_vector(), global_transfrom.up_vector());
	}

	Matrix4f CameraComponent::view_matrix(const Vector3D& position, const Vector3D& direction, const Vector3D& up_vector)
	{
		return glm::lookAt(position, position + direction, up_vector);
	}

	const CameraComponent& CameraComponent::camera_view(CameraView& out) const
	{
		auto& global_transform = world_transform();
		out.location           = global_transform.location();
		out.rotation           = global_transform.rotation();
		out.up_vector          = global_transform.up_vector();
		out.right_vector       = global_transform.right_vector();
		out.forward_vector     = global_transform.forward_vector();
		out.projection_mode    = projection_mode;
		out.fov                = fov;
		out.ortho_width        = ortho_width;
		out.ortho_height       = ortho_height;
		out.near_clip_plane    = near_clip_plane;
		out.far_clip_plane     = far_clip_plane;
		out.aspect_ratio       = aspect_ratio;

		return *this;
	}

	CameraView CameraComponent::camera_view() const
	{
		CameraView view;
		camera_view(view);
		return view;
	}

	CameraView& CameraView::operator=(class CameraComponent* component)
	{
		if (component)
		{
			component->camera_view(*this);
		}
		return *this;
	}

	CameraView& CameraView::operator=(class CameraComponent& component)
	{
		component.camera_view(*this);
		return *this;
	}
}// namespace Engine
