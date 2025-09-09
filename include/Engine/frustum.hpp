#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct CameraView;

	struct ENGINE_EXPORT Plane {
		Vector3f normal;
		float offset;

		Plane();
		Plane(const Vector4f& normal_distance);
		Plane(const Vector3f& normal, float distance);
		Plane(const Vector3f& normal, const Vector3f& location);

		Plane& normalize();
		float distance_to(const Vector3f& point) const;
		float distance_to(const Box3f& box) const;
	};

	struct ENGINE_EXPORT Frustum {
		Plane top;
		Plane bottom;
		Plane right;
		Plane left;
		Plane far;
		Plane near;

		Frustum();
		Frustum(const Matrix4f& projview);
		Frustum& operator=(const Matrix4f& projview);

		bool contains(const Box3f& box) const;
		bool contains(const Vector3f& point);
	};
}// namespace Engine
