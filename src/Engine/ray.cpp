#include <Engine/ray.hpp>


namespace Engine
{
	Ray::Ray(const Vector3D& origin, const Vector3D& direction) : m_origin(origin), m_direction(glm::normalize(direction))
	{}

	const Vector3D& Ray::origin() const
	{
		return m_origin;
	}

	const Vector3D& Ray::direction() const
	{
		return m_direction;
	}

	Ray& Ray::origin(const Vector3D& origin)
	{
		m_origin = origin;
		return *this;
	}

	Ray& Ray::direction(const Vector3D& direction)
	{
		m_direction = glm::normalize(direction);
		return *this;
	}
}// namespace Engine
