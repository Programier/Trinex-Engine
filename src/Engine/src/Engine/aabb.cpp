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
        return m_min;
    }

    const Vector3D& AABB_3Df::max() const
    {
        return m_max;
    }

    Vector3D AABB_3Df::center() const
    {
        return m_min + (m_max - m_min) / 2.0f;
    }

    AABB_3Df& AABB_3Df::center(const Vector3D& position)
    {
        Vector3D half_size = m_max - center();
        m_max             = position + half_size;
        m_min             = position - half_size;
        return *this;
    }

    AABB_3Df& AABB_3Df::min(const Vector3D& new_min)
    {
        return minmax(new_min, m_max);
    }

    AABB_3Df& AABB_3Df::max(const Vector3D& new_max)
    {
        return minmax(m_min, new_max);
    }

    AABB_3Df& AABB_3Df::minmax(const Vector3D& new_min, const Vector3D& new_max)
    {
        m_min = glm::min(new_min, new_max);
        m_max = glm::max(new_min, new_max);

        return *this;
    }

    Vector3D AABB_3Df::size() const
    {
        return m_max - m_min;
    }

    bool AABB_3Df::inside(const AABB_3Df& other) const
    {
        return (m_min.x >= other.m_min.x && m_max.x <= other.m_max.x) &&
               (m_min.y >= other.m_min.y && m_max.y <= other.m_max.y) &&
               (m_min.z >= other.m_min.z && m_max.z <= other.m_max.z);
    }

    bool AABB_3Df::intersect(const AABB_3Df& other) const
    {
        return (m_min.x <= other.m_max.x && m_max.x >= other.m_min.x) &&
               (m_min.y <= other.m_max.y && m_max.y >= other.m_min.y) &&
               (m_min.z <= other.m_max.z && m_max.z >= other.m_min.z);
    }

    Vector2D AABB_3Df::intersect(const Ray& ray) const
    {
        const Vector3D& origin    = ray.origin();
        const Vector3D& direction = ray.direction();

        Vector3D t_min = (m_min - origin) / direction;
        Vector3D t_max = (m_max - origin) / direction;
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
        return (m_min.x > other.m_max.x || m_max.x < other.m_min.x) ||
               (m_min.y > other.m_max.y || m_max.y < other.m_min.y) ||
               (m_min.z > other.m_max.z || m_max.z < other.m_min.z);
    }

    bool AABB_3Df::contains(const Vector3D& point) const
    {
        return point.x > m_min.x && point.x < m_max.x &&//
               point.y > m_min.y && point.y < m_max.y &&//
               point.z > m_min.z && point.z < m_max.z;
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
        m_max += offset;
        m_min += offset;
        return *this;
    }

    AABB_3Df& AABB_3Df::operator-=(const Vector3D& offset)
    {
        m_max -= offset;
        m_min -= offset;
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
        Vector3D tmp = self.m_max;
        self.m_max  = offset - self.m_min;
        self.m_min  = offset - tmp;
        return self;
    }

}// namespace Engine
