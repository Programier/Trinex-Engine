#include <Core/math/box.hpp>
#include <Core/math/math.hpp>
#include <Core/math/plane.hpp>

namespace Trinex::Math
{
	ENGINE_EXPORT Plane normalize(const Plane& plane)
	{
		const float length = Math::length(plane.normal);

		if (length != 0.f)
		{
			const float rcp = 1.0f / length;
			return Plane(plane.normal * rcp, plane.offset * rcp);
		}

		return plane;
	}

	ENGINE_EXPORT float distance(const Plane& plane, const Vector3f& point)
	{
		return Math::dot(point, plane.normal) + plane.offset;
	}

	ENGINE_EXPORT float distance(const Plane& plane, const Box3f& box)
	{
		return Math::dot(plane.normal, box.center()) + Math::dot(abs(plane.normal), box.extents()) + plane.offset;
	}

	ENGINE_EXPORT Plane left_plane(const Matrix4f& projview)
	{
		Vector4f row0 = Math::row(projview, 0);
		Vector4f row3 = Math::row(projview, 3);
		return Plane(row3 + row0);
	}

	ENGINE_EXPORT Plane right_plane(const Matrix4f& projview)
	{
		Vector4f row0 = Math::row(projview, 0);
		Vector4f row3 = Math::row(projview, 3);
		return Plane(row3 - row0);
	}

	ENGINE_EXPORT Plane top_plane(const Matrix4f& projview)
	{
		Vector4f row1 = Math::row(projview, 1);
		Vector4f row3 = Math::row(projview, 3);
		return Plane(row3 + row1);
	}

	ENGINE_EXPORT Plane bottom_plane(const Matrix4f& projview)
	{
		Vector4f row1 = Math::row(projview, 1);
		Vector4f row3 = Math::row(projview, 3);
		return Plane(row3 - row1);
	}

	ENGINE_EXPORT Plane near_plane(const Matrix4f& projview)
	{
		Vector4f row2 = Math::row(projview, 2);
		Vector4f row3 = Math::row(projview, 3);
		return Plane(row3 + row2);
	}

	ENGINE_EXPORT Plane far_plane(const Matrix4f& projview)
	{
		Vector4f row2 = Math::row(projview, 2);
		Vector4f row3 = Math::row(projview, 3);
		return Plane(row3 - row2);
	}
}// namespace Trinex::Math
