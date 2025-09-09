#include <Core/constants.hpp>
#include <Core/math/box.hpp>
#include <Core/math/math.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/frustum.hpp>


namespace Engine
{
	Plane::Plane() : normal(Constants::zero_vector), offset(0.f) {}
	Plane::Plane(const Vector4f& normal_distance) : Plane(static_cast<const Vector3f&>(normal_distance), normal_distance.w) {}

	Plane::Plane(const Vector3f& normal, float distance)
	{
		const float len = Math::length(normal);
		this->normal    = normal / len;
		this->offset    = distance / len;
	}

	Plane::Plane(const Vector3f& normal, const Vector3f& location)
	    : normal(Math::normalize(normal)), offset(-Math::dot(this->normal, location))
	{}

	Plane& Plane::normalize()
	{
		const float length = Math::length(normal);

		if (length != 0.f)
		{
			const float rcp = 1.0f / length;
			normal *= rcp;
			offset *= rcp;
		}

		return *this;
	}

	float Plane::distance_to(const Vector3f& point) const
	{
		return Math::dot(point, normal) + offset;
	}

	float Plane::distance_to(const Box3f& box) const
	{
		Vector3f extents     = box.extents();
		const float distance = distance_to(box.center());
		const float r = extents.x * Math::abs(normal.x) + extents.y * Math::abs(normal.y) + extents.z * Math::abs(normal.z);
		return distance + r;
	}

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
		new (&bottom) Plane(row3 + row1);
		new (&top) Plane(row3 - row1);
		new (&near) Plane(row3 + row2);
		new (&far) Plane(row3 - row2);

		return *this;
	}

	bool Frustum::contains(const Box3f& box) const
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (plane->distance_to(box) < 0.f)
				return false;
		}

		return true;
	}

	bool Frustum::contains(const Vector3f& point)
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (plane->distance_to(point) < 0.f)
				return false;
		}

		return true;
	}
}// namespace Engine
