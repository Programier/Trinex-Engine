#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class AABB_3Df;

	class ENGINE_EXPORT Ray final
	{
	private:
		Vector3f m_origin;
		Vector3f m_direction;

	public:
		Ray(const Vector3f& origin = {0.f, 0.f, 0.f}, const Vector3f& direction = {0.0f, 0.0f, 0.0f});
		const Vector3f& origin() const;
		const Vector3f& direction() const;
		Ray& origin(const Vector3f& origin);
		Ray& direction(const Vector3f& direction);

		bool intersect(const AABB_3Df&) const;
	};
}// namespace Engine
