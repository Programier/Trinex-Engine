#pragma once

#include <Core/export.hpp>
#include <Core/object.hpp>
#include <Core/engine_types.hpp>
#include <Core/constants.hpp>

namespace Engine
{
    class Ray final : public Object
    {
    private:
        Point3D _M_origin;
        Vector3D _M_direction;

    public:
        Ray(const Point3D& origin = Constants::zero_vector, const Vector3D& direction = Constants::zero_vector);
        const Point3D& origin() const;
        const Point3D& direction() const;
        Ray& origin(const Point3D& origin );
        Ray& direction(const Vector3D& direction);
    };
}
