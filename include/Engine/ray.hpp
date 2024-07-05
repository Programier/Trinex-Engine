#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class AABB_3Df;

    class ENGINE_EXPORT Ray final
    {
    private:
        Vector3D m_origin;
        Vector3D m_direction;

    public:
        Ray(const Vector3D& origin = {0.f, 0.f, 0.f}, const Vector3D& direction = {0.0f, 0.0f, 0.0f});
        const Point3D& origin() const;
        const Point3D& direction() const;
        Ray& origin(const Point3D& origin);
        Ray& direction(const Vector3D& direction);

        Vector2D intersect(const AABB_3Df&) const;
    };
}// namespace Engine
