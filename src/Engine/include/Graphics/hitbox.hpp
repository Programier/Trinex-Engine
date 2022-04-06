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
        float distance_to(const IHitBox& hitbox);
        const glm::vec3& rotation() const;
        const glm::vec3& position() const;
        const HitBoxType& type() const;
        const glm::vec3& size() const;

        glm::vec3& rotation();
        glm::vec3& position();
        glm::vec3& size();
    };

    class PointHB : public IHitBox
    {
    public:
        PointHB(const glm::vec3& position);
        PointHB(const PointHB&);
        PointHB& operator=(const PointHB& point);
    };

    class BoxHB : public IHitBox
    {
    public:
        BoxHB(const glm::vec3& position, const glm::vec3& size, const glm::vec3& rotation = {0.f, 0.f, 0.f});
        BoxHB(const BoxHB&);
        BoxHB& operator=(const BoxHB& box);
    };

    class SphereHB : public IHitBox
    {

    public:
        SphereHB(const glm::vec3& position, const float& radius);
        SphereHB(const SphereHB&);
        SphereHB& operator=(const SphereHB& sphere);
    };

    class CylinderHB : public IHitBox
    {
    public:
        CylinderHB(const glm::vec3& position, const float& radius, const float& height);
        CylinderHB(const CylinderHB&);
        CylinderHB& operator=(const CylinderHB& point);
    };
}// namespace Engine
