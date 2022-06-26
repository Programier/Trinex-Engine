#pragma once
#include <Graphics/basic_object.hpp>
#include <Graphics/line.hpp>


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

    class IHitBox : public BasicObject<Rotate, Translate, Scale>
    {
    protected:
        HitBoxType _M_type;
        Size3D _M_size;
        Line _M_lines;

    public:
        Distance distance_to(const IHitBox& hitbox);
        const HitBoxType& type() const;
        const Size3D& size() const;
        glm::vec3& size();
        Line& lines();
    };

    class PointHB : public IHitBox
    {
    public:
        PointHB(const Point3D& position);
        PointHB(const PointHB&);
        PointHB& operator=(const PointHB& point);
    };

    class BoxHB : public IHitBox
    {
    public:
        BoxHB(const Point3D& position = {0.f, 0.f, 0.f}, const Size3D& size = {1.f, 1.f, 1.f},
              const EulerAngle3D& rotation = {0.f, 0.f, 0.f});
        BoxHB(const BoxHB&);
        BoxHB& operator=(const BoxHB& box);
    };

    class SphereHB : public IHitBox
    {

    public:
        SphereHB(const Point3D& position, const float& radius);
        SphereHB(const SphereHB&);
        SphereHB& operator=(const SphereHB& sphere);
    };

    class CylinderHB : public IHitBox
    {
    public:
        CylinderHB(const glm::vec3& position, const float& radius, const Size1D& height);
        CylinderHB(const CylinderHB&);
        CylinderHB& operator=(const CylinderHB& point);
    };
}// namespace Engine
