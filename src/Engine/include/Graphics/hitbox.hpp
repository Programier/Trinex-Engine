#pragma once

#include <Graphics/line.hpp>
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

    class PointHB : public IHitBox
    {
    public:
        PointHB(const glm::vec3& position);
        PointHB(const PointHB&);
        PointHB& operator=(const PointHB& point);
        float distance_to(const IHitBox& hitbox) override;
    };

    class BoxHB : public IHitBox
    {
    private:
        Line _M_lines;

    public:
        BoxHB(const glm::vec3& position, const glm::vec3& size);
        BoxHB(const BoxHB&);
        BoxHB& operator=(const BoxHB& box);
        float distance_to(const IHitBox& hitbox) override;

        BoxHB& draw(const float& width = 2.0f);
    };

    class SphereHB : public IHitBox
    {

    public:
        SphereHB(const glm::vec3& position);
        SphereHB(const SphereHB&);
        SphereHB& operator=(const SphereHB& sphere);
        float distance_to(const IHitBox& hitbox) override;
    };

    class CylinderHB : public IHitBox
    {
    public:
        CylinderHB(const glm::vec3& position);
        CylinderHB(const CylinderHB&);
        CylinderHB& operator=(const CylinderHB& point);
        float distance_to(const IHitBox& hitbox) override;
    };
}// namespace Engine
