#include <Engine/aabb.hpp>
#include <Engine/ray.hpp>


namespace Engine
{
    AABB_3Df::AABB_3Df(const Vector3D& min, const Vector3D& max)
    {
        minmax(min, max);
    }

    AABB_3Df::AABB_3Df(const AABB_3Df& other)            = default;
    AABB_3Df& AABB_3Df::operator=(const AABB_3Df& other) = default;

    const Vector3D& AABB_3Df::min() const
    {
        return _M_min;
    }

    const Vector3D& AABB_3Df::max() const
    {
        return _M_max;
    }

    Vector3D AABB_3Df::center() const
    {
        return _M_min + (_M_max - _M_min) / 2.0f;
    }

    AABB_3Df& AABB_3Df::min(const Vector3D& new_min)
    {
        return minmax(new_min, _M_max);
    }

    AABB_3Df& AABB_3Df::max(const Vector3D& new_max)
    {
        return minmax(_M_min, new_max);
    }

    AABB_3Df& AABB_3Df::minmax(const Vector3D& new_min, const Vector3D& new_max)
    {
        _M_min = glm::min(new_min, new_max);
        _M_max = glm::max(new_min, new_max);

        return *this;
    }

    bool AABB_3Df::inside(const AABB_3Df& other) const
    {
        return (_M_min.x >= other._M_min.x && _M_max.x <= other._M_max.x) &&
               (_M_min.y >= other._M_min.y && _M_max.y <= other._M_max.y) &&
               (_M_min.z >= other._M_min.z && _M_max.z <= other._M_max.z);
    }

    bool AABB_3Df::intersect(const AABB_3Df& other) const
    {
        return (_M_min.x <= other._M_max.x && _M_max.x >= other._M_min.x) &&
               (_M_min.y <= other._M_max.y && _M_max.y >= other._M_min.y) &&
               (_M_min.z <= other._M_max.z && _M_max.z >= other._M_min.z);
    }

    Vector2D AABB_3Df::intersect(const Ray& ray) const
    {
        const Vector3D& origin    = ray.origin();
        const Vector3D& direction = ray.direction();

        Vector3D t_min = (_M_min - origin) / direction;
        Vector3D t_max = (_M_max - origin) / direction;
        Vector3D t1    = glm::min(t_min, t_max);
        Vector3D t2    = glm::max(t_min, t_max);
        float t_near   = glm::max(glm::max(t1.x, t1.y), t1.z);
        float t_far    = glm::min(glm::min(t2.x, t2.y), t2.z);
        return Vector2D(t_near, t_far);
    }

    Vector2D Ray::intersect(const AABB_3Df& aabb) const
    {
        return aabb.intersect(*this);
    }

    bool AABB_3Df::outside(const AABB_3Df& other) const
    {
        return (_M_min.x > other._M_max.x || _M_max.x < other._M_min.x) ||
               (_M_min.y > other._M_max.y || _M_max.y < other._M_min.y) ||
               (_M_min.z > other._M_max.z || _M_max.z < other._M_min.z);
    }
}// namespace Engine
