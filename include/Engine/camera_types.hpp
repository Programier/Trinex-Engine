#pragma once
#include <Core/engine_types.hpp>
#include <Core/transform.hpp>

namespace Engine
{
	enum class CameraProjectionMode
	{
		Perspective  = 0,
		Orthographic = 1,
	};

	struct ENGINE_EXPORT CameraView {
		Vector3f location;
		Quaternion rotation;
		Vector3f forward_vector;
		Vector3f up_vector;
		Vector3f right_vector;

		CameraProjectionMode projection_mode;
		float fov;
		float ortho_width;
		float ortho_height;
		float near_clip_plane;
		float far_clip_plane;
		float aspect_ratio;

		Matrix4f projection_matrix() const;
		Matrix4f view_matrix() const;
		static ENGINE_EXPORT Matrix4f view_matrix(const Vector3f& position, const Vector3f& direction, const Vector3f& up_vector);

		CameraView& operator=(class CameraComponent*);
		CameraView& operator=(class CameraComponent&);
	};
}// namespace Engine
