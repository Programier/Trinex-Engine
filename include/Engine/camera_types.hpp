#pragma once
#include <Core/math/math.hpp>
#include <Core/math/quaternion.hpp>
#include <Core/transform.hpp>
#include <Engine/enums.hpp>

namespace Engine
{
	struct ENGINE_EXPORT CameraView {
		struct Ortho {
			float left;
			float right;
			float top;
			float bottom;
		};

		struct Perspective {
			float fov;
			float aspect_ratio;
		};

		CameraProjectionMode projection_mode;

		Vector3f location;
		Vector3f forward;
		Vector3f up;
		Vector3f right;

		float near;
		float far;

		union
		{
			Ortho ortho;
			Perspective perspective;
		};

		Matrix4f projection_matrix() const;
		Matrix4f view_matrix() const;
		float linearize_depth(float depth) const;
		Vector3f reconstruct_position_ndc(Vector2f ndc, float depth) const;
		inline Vector3f reconstruct_position(Vector2f uv, float depth) const
		{
			return reconstruct_position_ndc(uv * 2.f - 1.f, depth);
		}

		inline uint_t compute_lod(const Vector3f& object_location, uint_t lod_count) const
		{
			if (lod_count <= 1)
				return 0;

			float_t distance   = Math::distance(object_location, location);
			float_t lod_factor = ((far - near) * 0.75f) / lod_count;
			uint_t lod_index   = static_cast<uint_t>((distance - near) / lod_factor);
			return glm::min<uint_t>(lod_index, lod_count - 1);
		}

		CameraView& operator=(class CameraComponent*);
		CameraView& operator=(class CameraComponent&);
	};
}// namespace Engine
