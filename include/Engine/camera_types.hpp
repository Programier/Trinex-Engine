#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/quaternion.hpp>
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
		float linearize_depth(float depth) const;
		Vector3f reconstruct_position_ndc(Vector2f ndc, float depth) const;
		inline Vector3f reconstruct_position(Vector2f uv, float depth) const
		{
			return reconstruct_position_ndc(uv * 2.f - 1.f, depth);
		}

		static ENGINE_EXPORT Matrix4f view_matrix(const Vector3f& position, const Vector3f& direction, const Vector3f& up_vector);

		inline uint_t compute_lod(const Vector3f& object_location, uint_t lod_count) const
		{
			if (lod_count <= 1)
				return 0;

			float_t distance   = glm::distance(object_location, location);
			float_t lod_factor = ((far_clip_plane - near_clip_plane) * 0.75f) / lod_count;
			uint_t lod_index   = static_cast<uint_t>((distance - near_clip_plane) / lod_factor);
			return glm::min<uint_t>(lod_index, lod_count - 1);
		}

		CameraView& operator=(class CameraComponent*);
		CameraView& operator=(class CameraComponent&);
	};
}// namespace Engine
