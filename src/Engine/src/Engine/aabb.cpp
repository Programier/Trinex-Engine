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

    AABB_3Df& AABB_3Df::center(const Vector3D& position)
    {
        Vector3D half_size = _M_max - center();
        _M_max             = position + half_size;
        _M_min             = position - half_size;
        return *this;
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

    Vector3D AABB_3Df::size() const
    {
        return _M_max - _M_min;
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

    bool AABB_3Df::contains(const Vector3D& point) const
    {
        return point.x > _M_min.x && point.x < _M_max.x &&//
               point.y > _M_min.y && point.y < _M_max.y &&//
               point.z > _M_min.z && point.z < _M_max.z;
    }

    template<typename T>
    static AABB_3Df mult_op(const AABB_3Df& self, const T& value)
    {
        Vector3D box_center = self.center();
        Vector3D offset     = (self.max() - box_center) * value;
        return AABB_3Df(box_center - offset, box_center + offset);
    }

    template<typename T>
    static AABB_3Df div_op(const AABB_3Df& self, const T& value)
    {
        Vector3D box_center = self.center();
        Vector3D offset     = (self.max() - box_center) / value;
        return AABB_3Df(box_center - offset, box_center + offset);
    }

    AABB_3Df AABB_3Df::operator*(float value) const
    {
        return mult_op(*this, value);
    }

    AABB_3Df AABB_3Df::operator/(float value) const
    {
        return div_op(*this, value);
    }

    AABB_3Df AABB_3Df::operator*(const Vector3D& scale) const
    {
        return mult_op(*this, scale);
    }

    AABB_3Df AABB_3Df::operator/(const Vector3D& scale) const
    {
        return div_op(*this, scale);
    }

    AABB_3Df& AABB_3Df::operator+=(const Vector3D& offset)
    {
        _M_max += offset;
        _M_min += offset;
        return *this;
    }

    AABB_3Df& AABB_3Df::operator-=(const Vector3D& offset)
    {
        _M_max -= offset;
        _M_min -= offset;
        return *this;
    }

    AABB_3Df AABB_3Df::operator+(const Vector3D& offset) const
    {
        AABB_3Df new_box = (*this);
        return new_box += offset;
    }

    AABB_3Df AABB_3Df::operator-(const Vector3D& offset) const
    {
        AABB_3Df new_box = (*this);
        return new_box -= offset;
    }

    ENGINE_EXPORT AABB_3Df operator*(float value, const AABB_3Df& box)
    {
        return box * value;
    }

    ENGINE_EXPORT AABB_3Df operator*(const Vector3D& scale, const AABB_3Df& box)
    {
        return box * scale;
    }

    ENGINE_EXPORT AABB_3Df operator+(const Vector3D& offset, AABB_3Df self)
    {
        return self += offset;
    }

    ENGINE_EXPORT AABB_3Df operator-(const Vector3D& offset, AABB_3Df self)
    {
        Vector3D tmp = self._M_max;
        self._M_max  = offset - self._M_min;
        self._M_min  = offset - tmp;
        return self;
    }

}// namespace Engine
