#pragma once
#include <Core/math/plane.hpp>

namespace Trinex
{
	struct ENGINE_EXPORT Frustum {
		Plane left;
		Plane right;
		Plane top;
		Plane bottom;
		Plane near;
		Plane far;

		Frustum();
		Frustum(const Matrix4f& projview);
		Frustum& operator=(const Matrix4f& projview);

		bool contains(const Vector3f& point);
		bool contains(const Box3f& box) const;
		bool intersects(const Box3f& box) const;
	};
}// namespace Trinex
