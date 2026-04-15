#pragma once
#include <Core/math/vector.hpp>

namespace Trinex
{
	struct ENGINE_EXPORT Plane {
		Vector3f normal;
		f32 offset;

		Plane();
		Plane(const Vector4f& equation);
		Plane(const Vector3f& normal, float distance);
		Plane(const Vector3f& normal, const Vector3f& location);
	};
}// namespace Trinex
