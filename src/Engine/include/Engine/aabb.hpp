#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class Ray;

    class ENGINE_EXPORT AABB_3Df
    {
    private:
        Vector3D _M_min;
        Vector3D _M_max;


    public:
        AABB_3Df(const Vector3D& min = {}, const Vector3D& max = {});
        AABB_3Df(const AABB_3Df& other);
        AABB_3Df& operator = (const AABB_3Df& other);

        const Vector3D& min() const;
        const Vector3D& max() const;
        Vector3D center() const;

        AABB_3Df& min(const Vector3D& new_min);
        AABB_3Df& max(const Vector3D& new_max);
        AABB_3Df& minmax(const Vector3D& new_min, const Vector3D& new_max);

        bool inside(const AABB_3Df& other) const;
        bool intersect(const AABB_3Df& other) const;
        Vector2D intersect(const Ray& ray) const;
        bool outside(const AABB_3Df& other) const;
    };
}// namespace Engine
