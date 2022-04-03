#pragma once

#include <glm/glm.hpp>


namespace Engine
{
#define HITBOX_TYPES_COUNT 4
    enum class HitBoxType
    {
        POINT,
        BOX,
        SPHERE,
        CYLINDER
    };

    class IHitBox
    {
    protected:
        glm::vec3 _M_rotate = {0.f, 0.f, 0.f};
        glm::vec3 _M_position;
        HitBoxType _M_type;
        glm::vec3 _M_size;

    public:
        virtual float distance_to(const IHitBox& hitbox) = 0;
        const glm::vec3& rotation() const;
        const glm::vec3& position() const;
        const HitBoxType& type() const;
        const glm::vec3& size() const;

        glm::vec3& rotation();
        glm::vec3& position();
        glm::vec3& size();
    };

    class Point : public IHitBox
    {
    public:
        Point(const glm::vec3& position);
        Point(const Point&);
        Point& operator = (const Point& point);
        float distance_to(const IHitBox& hitbox) override;
    };

    class Box : public IHitBox
    {
    public:
        Box(const glm::vec3& position);
        Box(const Box&);
        Box& operator = (const Box& box);
        float distance_to(const IHitBox& hitbox) override;
    };

    class Sphere : public IHitBox
    {
    public:
        Sphere(const glm::vec3& position);
        Sphere(const Sphere&);
        Sphere& operator = (const Sphere& sphere);
        float distance_to(const IHitBox& hitbox) override;
    };

    class Cylinder : public IHitBox
    {
    public:
        Cylinder(const glm::vec3& position);
        Cylinder(const Cylinder&);
        Cylinder& operator = (const Cylinder& point);
        float distance_to(const IHitBox& hitbox) override;
    };
}// namespace Engine
