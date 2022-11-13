#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Graphics/camera.hpp>


namespace Engine
{
    STRUCT Plane {
        Vector3D normal = Constants::zero_vector;
        float distance = 0.f;

        Plane();
        Plane(const Vector3D& p1, const Vector3D& norm);
        float get_signed_distance_to_plan(const Point3D& point) const;
    };

    STRUCT Frustum
    {
        Plane top;
        Plane bottom;

        Plane right;
        Plane left;

        Plane far;
        Plane near;

        Frustum(const Camera& camera);
        Frustum& from_camera(const Camera& camera);
    };
}// namespace Engine
