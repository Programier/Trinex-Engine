#include <Core/logger.hpp>
#include <Graphics/hitbox.hpp>
#include <iostream>


namespace Engine
{
    HitBox::~HitBox() = default;

    implement_class_cpp(BoxHB);
    BoxHB::BoxHB(const AABB_3D& box)
    {
        aabb(box);
    }

    const AABB_3D& BoxHB::aabb() const
    {
        return _M_aabb;
    }

    BoxHB& BoxHB::aabb(const AABB_3D& box)
    {
        _M_aabb.min = glm::min(box.min, box.max);
        _M_aabb.max = glm::max(box.min, box.max);
        _M_center = (_M_aabb.max + _M_aabb.min) / 2.f;
        _M_half_sizes = _M_aabb.max - _M_center;
        return *this;
    }
    bool BoxHB::is_on_or_forward_plan(const Plane& plane) const
    {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = _M_half_sizes.x * std::abs(plane.normal.x) + _M_half_sizes.y * std::abs(plane.normal.y) +
                        _M_half_sizes.z * std::abs(plane.normal.z);

        return -r <= plane.get_signed_distance_to_plan(_M_center);
    }


    bool BoxHB::is_in_frustum(const Frustum& frustum, const glm::mat4& model) const
    {
        //Get global scale thanks to our transform
        AABB_3D global = _M_aabb;

        global.max = model * Vector4D(global.max, 1.f);
        global.min = model * Vector4D(global.min, 1.f);

        BoxHB hitbox(global);


        return (hitbox.is_on_or_forward_plan(frustum.left) && hitbox.is_on_or_forward_plan(frustum.right) &&
                hitbox.is_on_or_forward_plan(frustum.top) && hitbox.is_on_or_forward_plan(frustum.bottom) &&
                hitbox.is_on_or_forward_plan(frustum.near) && hitbox.is_on_or_forward_plan(frustum.far));
    }

    void BoxHB::render(const glm::mat4& model)
    {
        throw not_implemented;
    }
}// namespace Engine
