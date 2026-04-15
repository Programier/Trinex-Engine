#include <Core/math/box.hpp>
#include <Core/math/frustum.hpp>
#include <Core/math/math.hpp>

namespace Trinex
{
	Frustum::Frustum() = default;

	Frustum::Frustum(const Matrix4f& projview)
	{
		(*this) = projview;
	}

	Frustum& Frustum::operator=(const Matrix4f& projview)
	{
		Vector4f row0 = Math::row(projview, 0);
		Vector4f row1 = Math::row(projview, 1);
		Vector4f row2 = Math::row(projview, 2);
		Vector4f row3 = Math::row(projview, 3);

		new (&left) Plane(row3 + row0);
		new (&right) Plane(row3 - row0);
		new (&top) Plane(row3 + row1);
		new (&bottom) Plane(row3 - row1);
		new (&near) Plane(row3 + row2);
		new (&far) Plane(row3 - row2);

		return *this;
	}

	bool Frustum::contains(const Box3f& box) const
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			const float distance = Math::distance(*plane, box.center());
			const Vector3f& n    = plane->normal;
			const Vector3f e     = box.extents();
			const float r        = e.x * Math::abs(n.x) + e.y * Math::abs(n.y) + e.z * Math::abs(n.z);

			if (distance - r < 0.f)
				return false;
		}
		return true;
	}

	bool Frustum::contains(const Vector3f& point)
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (Math::distance(*plane, point) < 0.f)
				return false;
		}

		return true;
	}

	bool Frustum::intersects(const Box3f& box) const
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (Math::distance(*plane, box) < 0.f)
				return false;
		}

		return true;
	}
}// namespace Trinex
