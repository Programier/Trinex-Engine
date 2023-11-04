#include <Graphics/frustum.hpp>
#include <iostream>


namespace Engine
{
    Plane::Plane() = default;
    Plane::Plane(const Vector3D& _p1, const Vector3D& _normal)
        : normal(glm::normalize(_normal)), distance(glm::dot(normal, _p1))
    {}

    float Plane::get_signed_distance_to_plan(const Point3D& point) const
    {
        return glm::dot(normal, point) - distance;
    }

    Frustum::Frustum(const Camera& camera)
    {
        from_camera(camera);
    }

    Frustum& Frustum::from_camera(const Camera& camera)
    {
        const Transform& transform = camera.transform;
        const auto pos             = transform.position;

        const float _near = camera.min_render_distance();

        const float view_angle = camera.viewing_angle();
        const float aspect     = camera.aspect();
        if (aspect == 0)
            throw std::runtime_error("Aspect can't be zero!");

        const auto front_vector = transform.forward_vector();
        const auto up_vector    = transform.up_vector();
        const auto right_vector = transform.right_vector();


        const float half_v_side = camera.max_render_distance() * glm::tan(view_angle * 0.5f);
        const float half_h_side = half_v_side * aspect;

        const Vector3D front_mult_far = camera.max_render_distance() * front_vector;

        near = {pos + _near * front_vector, front_vector};
        far  = {pos + front_mult_far, -front_vector};

        right  = {pos, glm::cross(up_vector, front_mult_far + right_vector * half_h_side)};
        left   = {pos, glm::cross(front_mult_far - right_vector * half_h_side, up_vector)};
        top    = {pos, glm::cross(right_vector, front_mult_far - up_vector * half_v_side)};
        bottom = {pos, glm::cross(front_mult_far + up_vector * half_v_side, right_vector)};
        return *this;
    }
}// namespace Engine
