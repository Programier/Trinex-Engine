#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/camera_types.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/window.hpp>

namespace Engine
{
	Matrix4f CameraView::projection_matrix() const
	{
		if (projection_mode == CameraProjectionMode::Perspective)
		{
			Matrix4f projection = Math::perspective(Math::radians(perspective.fov), perspective.aspect_ratio, near, far);
			projection[1][1] *= -1;
			return projection;
		}
		else if (projection_mode == CameraProjectionMode::Orthographic)
		{
			return Math::ortho(ortho.left, ortho.right, ortho.top, ortho.bottom, near, far);
		}

		return Matrix4f(1.f);
	}

	Matrix4f CameraView::view_matrix() const
	{
		return Math::look_at(location, location + forward, up);
	}

	float CameraView::linearize_depth(float depth) const
	{
		return Math::linearize_depth(depth, near, far);
	}

	Vector3f CameraView::reconstruct_position_ndc(Vector2f ndc, float depth) const
	{
		Vector4f ndc_pos  = Vector4f(ndc.x, ndc.y, depth, 1.0f);
		Matrix4f inv_proj = glm::inverse(projection_matrix() * view_matrix());
		Vector4f view_pos = inv_proj * ndc_pos;
		view_pos /= view_pos.w;
		return {view_pos.x, view_pos.y, view_pos.z};
	}

	trinex_implement_engine_class_default_init(CameraComponent, 0);

	bool CameraComponent::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		return archive.serialize(projection_mode, fov, ortho_width, ortho_height, near, far, aspect_ratio);
	}

	Matrix4f CameraComponent::projection_matrix()
	{
		if (projection_mode == CameraProjectionMode::Perspective)
		{
			return Math::perspective(Math::radians(fov), aspect_ratio, near, far);
		}
		else if (projection_mode == CameraProjectionMode::Orthographic)
		{
			return Math::ortho(-ortho_width / 2.0f, // Left
			                   ortho_width / 2.0f,  // Right
			                   -ortho_height / 2.0f,// Bottom
			                   ortho_height / 2.0f, // Top
			                   near,                // Near clipping plane
			                   far                  // Far clipping plane
			);
		}

		return Matrix4f(1.0);
	}

	Matrix4f CameraComponent::view_matrix()
	{
		auto& global_transfrom = world_transform();
		return view_matrix(global_transfrom.location(), global_transfrom.forward_vector(), global_transfrom.up_vector());
	}

	Matrix4f CameraComponent::view_matrix(const Vector3f& position, const Vector3f& direction, const Vector3f& up_vector)
	{
		return glm::lookAt(position, position + direction, up_vector);
	}

	const CameraComponent& CameraComponent::camera_view(CameraView& out) const
	{
		auto& global_transform = world_transform();
		out.location           = global_transform.location();
		out.up                 = global_transform.up_vector();
		out.right              = global_transform.right_vector();
		out.forward            = global_transform.forward_vector();
		out.projection_mode    = projection_mode;

		if (projection_mode == CameraProjectionMode::Perspective)
		{
			out.perspective.fov          = fov;
			out.perspective.aspect_ratio = aspect_ratio;
		}
		else
		{
			out.ortho.left   = -ortho_width / 2.0f;
			out.ortho.right  = ortho_width / 2.0f;
			out.ortho.top    = ortho_height / 2.f;
			out.ortho.bottom = -ortho_height / 2.f;
		}

		out.near = near;
		out.far  = far;

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
