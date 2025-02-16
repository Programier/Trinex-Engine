#include <Engine/ray.hpp>


namespace Engine
{
	Ray::Ray(const Vector3f& origin, const Vector3f& direction) : m_origin(origin), m_direction(glm::normalize(direction))
	{}

	const Vector3f& Ray::origin() const
	{
		return m_origin;
	}

	const Vector3f& Ray::direction() const
	{
		return m_direction;
	}

	Ray& Ray::origin(const Vector3f& origin)
	{
		m_origin = origin;
		return *this;
	}

	Ray& Ray::direction(const Vector3f& direction)
	{
		m_direction = glm::normalize(direction);
		return *this;
	}
}// namespace Engine
