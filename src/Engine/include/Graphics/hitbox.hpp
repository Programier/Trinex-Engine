#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Graphics/frustum.hpp>


namespace Engine
{

    CLASS HitBox
    {
    public:
        virtual void render(const glm::mat4& model) = 0;
        virtual bool is_in_frustum(const Frustum& frustum, const glm::mat4& model) const = 0;
        virtual ~HitBox();
    };


    CLASS BoxHB : public HitBox
    {
    protected:
        AABB_3D _M_aabb;
        Point3D _M_center;
        Size3D _M_half_sizes;

    public:
        implement_class_hpp(BoxHB);
        BoxHB(const AABB_3D& box);
        const AABB_3D& aabb() const;
        BoxHB& aabb(const AABB_3D& box);
        bool is_on_or_forward_plan(const Plane& plan) const;
        bool is_in_frustum(const Frustum& frustum, const glm::mat4& model) const override;
        void render(const glm::mat4& model) override;
    };

    //    CLASS SphereHB : public HitBox
    //    {

    //    public:
    //        SphereHB(const Point3D& position, const float& radius);
    //        SphereHB(const SphereHB&);
    //        SphereHB& operator=(const SphereHB& sphere);
    //    };
}// namespace Engine
