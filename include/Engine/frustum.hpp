#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct CameraView;
	class AABB_3Df;

	struct ENGINE_EXPORT Plane {
		Vector3f normal;
		float distance;

		Plane();
		Plane(const Vector3f& p1, const Vector3f& norm);
		float signed_distance_to_plane(const Vector3f& point) const;
		bool is_on_or_forward(const Vector3f& point) const;
		bool is_on_or_forward(const AABB_3Df& box) const;
	};

	struct ENGINE_EXPORT Frustum {

		Plane top;
		Plane bottom;
		Plane right;
		Plane left;
		Plane far;
		Plane near;

		Frustum(const CameraView& camera);
		Frustum& operator=(const CameraView& view);

		bool in_frustum(const AABB_3Df& box) const;
	};
}// namespace Engine
