#include <Core/math/box.hpp>
#include <Core/math/math.hpp>
#include <Core/math/plane.hpp>

namespace Trinex
{
	Plane::Plane() : normal(0.f, 0.f, 0.f), offset(0.f) {}
	Plane::Plane(const Vector4f& equation) : Plane(static_cast<const Vector3f&>(equation), equation.w) {}

	Plane::Plane(const Vector3f& normal, float distance)
	{
		const float len = Math::length(normal);
		this->normal    = normal / len;
		this->offset    = distance / len;
	}

	Plane::Plane(const Vector3f& normal, const Vector3f& location)
	    : normal(Math::normalize(normal)), offset(-Math::dot(this->normal, location))
	{}
}// namespace Trinex
