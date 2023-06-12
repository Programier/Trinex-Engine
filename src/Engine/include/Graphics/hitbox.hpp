#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Graphics/frustum.hpp>
#include <Graphics/ray.hpp>
#include <Core/color.hpp>


namespace Engine
{

    class ENGINE_EXPORT HitBox
    {
    public:
        virtual void render(const glm::mat4& model = Constants::identity_matrix, const Color& color = Color::Red)
                const = 0;
        virtual bool is_in_frustum(const Frustum& frustum, const glm::mat4& model) const = 0;
        virtual bool is_in_frustum(const Frustum& frustum) const = 0;
        virtual Vector2D intersect(const Ray& ray) const = 0;
        virtual ~HitBox();
    };


    class ENGINE_EXPORT BoxHB : public HitBox, public AABB_3D
    {
    protected:
        Point3D _M_center;
        Size3D _M_half_sizes;

    public:
        BoxHB();
        BoxHB(const AABB_3D& box);
        BoxHB(const Size3D& _min, const Size3D& _max);
        copy_constructors_hpp(BoxHB);

        const AABB_3D& aabb() const;
        BoxHB& aabb(const AABB_3D& box);
        BoxHB& aabb(const BoxHB& box);
        BoxHB& aabb(const Size3D& _min, const Size3D& _max);
        bool is_on_or_forward_plan(const Plane& plan) const;
        bool is_in_frustum(const Frustum& frustum, const glm::mat4& model) const override;
        bool is_in_frustum(const Frustum& frustum) const override;
        void render(const glm::mat4& model = Constants::identity_matrix, const Color& color = Color::Red)
                const override;
        BoxHB apply_model(const glm::mat4& model) const;
        Vector2D intersect(const Ray& ray) const override;


        BoxHB max_box(const AABB_3D& box) const;
        bool contains(const AABB_3D& box) const;
        bool contains(const BoxHB& box) const;
        bool inside(const AABB_3D& box) const;
        bool inside(const BoxHB& box) const;
        const Point3D& center() const;
        const Size3D& half_size() const;
    };

    //    CLASS SphereHB : public HitBox
    //    {

    //    public:
    //        SphereHB(const Point3D& position, const float& radius);
    //        SphereHB(const SphereHB&);
    //        SphereHB& operator=(const SphereHB& sphere);
    //    };
}// namespace Engine
