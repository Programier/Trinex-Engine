#include <Graphics/ray.hpp>


namespace Engine
{
    Ray::Ray(const Point3D& origin, const Vector3D& direction) : _M_origin(origin), _M_direction(glm::normalize(direction))
    {}

    const Point3D& Ray::origin() const
    {
        return _M_origin;
    }

    const Point3D& Ray::direction() const
    {
        return _M_direction;
    }

    Ray& Ray::origin(const Point3D& origin)
    {
        _M_origin = origin;
        return *this;
    }

    Ray& Ray::direction(const Vector3D& direction)
    {
        _M_direction = glm::normalize(direction);
        return *this;
    }
}// namespace Engine
