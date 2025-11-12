#pragma once
#include <Core/math/math.hpp>
#include <Core/math/matrix.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct ENGINE_EXPORT CameraView {
		Matrix4f projection;
		Matrix4f view;
		Matrix4f projview;
		Matrix4f inv_projection;
		Matrix4f inv_view;
		Matrix4f inv_projview;

		float near;
		float far;

		static CameraView static_perspective(const Vector3f& origin, const Vector3f& forward, const Vector3f& up, float fov,
		                                     float aspect, float near, float far);
		static CameraView static_ortho(const Vector3f& origin, const Vector3f& forward, const Vector3f& up, float left,
		                               float right, float bottom, float top, float near, float far);

		CameraView& perspective(float fov, float aspect, float near, float far);
		CameraView& ortho(float left, float right, float bottom, float top, float near, float far);
		CameraView& look(const Vector3f& origin, const Vector3f& forward, const Vector3f& up);

		Vector3f right() const { return inv_view[0]; }
		Vector3f up() const { return inv_view[1]; }
		Vector3f forward() const { return inv_view[2]; }
		Vector3f location() const { return inv_view[3]; }

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

			float_t distance   = Math::distance(object_location, location());
			float_t lod_factor = ((far - near) * 0.75f) / lod_count;
			uint_t lod_index   = static_cast<uint_t>((distance - near) / lod_factor);
			return glm::min<uint_t>(lod_index, lod_count - 1);
		}
	};
}// namespace Engine
