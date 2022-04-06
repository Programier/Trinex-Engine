#include <Graphics/hitbox.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <vector>

#define in_range(value, min, max) (value >= min && value <= max)


// From point distance
static float from_point_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::distance(point1.position(), point2.position());
}

static glm::vec3 rotate_point(const glm::vec3& p, const glm::vec3& r)
{
    glm::mat4 result = glm::rotate(glm::mat4(1.0f), -r.x, {1, 0, 0});
    result = glm::rotate(result, -r.y, {0, 1, 0});
    result = glm::rotate(result, -r.z, {0, 0, 1});
    return glm::vec3(result * glm::vec4(p, 0.f));
}

static float from_point_to_box(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    auto point = rotate_point(point1.position(), point1.rotation());
    auto& cube = point2.position();
    auto& size = point2.size();

    auto min = cube - size;
    auto max = cube + size;

    auto vector = point - cube;

    int index = 0;
    int count = 0;
    float max_diff = 0;
    int s_index = -1;
    for (int i = 0; i < 3; i++)
    {
        if (glm::abs(vector[i]) <= size[i])
        {
            index += i;
            count++;
            if (glm::abs(max_diff) < glm::abs(vector[i]))
            {
                max_diff = vector[i];
                s_index = i;
            }
            vector[i] += cube[i];
        }
        else
            vector[i] = vector[i] < 0 ? min[i] : max[i];
    }
    return (count == 3)   ? glm::distance(point, cube) - (size[s_index] - max_diff)
           : (count == 2) ? glm::distance(point, cube) - size[3 - index]
                          : glm::distance(point, vector);
}

static float from_point_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::distance(point1.position(), point2.position()) - point2.size().r;
}

static float from_point_to_cylinder(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    auto point = rotate_point(point1.position(), point2.rotation());
    auto& size = point2.size();
    auto& cylinder = point2.position();
    auto vector = point - point2.position();

    if (glm::abs(vector[1]) <= size[1])
        return glm::distance(point, cylinder) - size.r;
    if ((vector[0] * vector[0] + vector[2] * vector[2]) <= (size.r * size.r))
        return glm::distance(point, cylinder) - size.y;
    float K = vector[2] == 0 ? 0 : vector[0] / vector[2];
    float X = size.r * glm::sqrt(1.0f / (1.0f + K * K));
    vector[0] = cylinder.x + X;
    vector[1] = vector[1] > 0 ? cylinder.y + size.y : cylinder.y - size.y;
    vector[2] = cylinder.z + X * K;
    return glm::distance(vector, point);
}

// From box distance

// From sphere distance
static float from_sphere_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_sphere(point2, point1);
}

static float from_sphere_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::abs(glm::distance(point1.position(), point2.position())) - point1.size().r - point2.size().r;
}


// From cylinder distance

static float (*distance_function_array[HITBOX_TYPES_COUNT][HITBOX_TYPES_COUNT])(const Engine::IHitBox&,
                                                                                const Engine::IHitBox&) = {
        {from_point_to_point, from_point_to_box, from_point_to_sphere, from_point_to_cylinder}};

namespace Engine
{
    // Hitbox interface implementation
    const glm::vec3& IHitBox::rotation() const
    {
        return _M_rotate;
    }

    const glm::vec3& IHitBox::position() const
    {
        return _M_position;
    }


    const HitBoxType& IHitBox::type() const
    {
        return _M_type;
    }

    const glm::vec3& IHitBox::size() const
    {
        return _M_size;
    }


    glm::vec3& IHitBox::rotation()
    {
        return _M_rotate;
    }

    glm::vec3& IHitBox::position()
    {
        return _M_position;
    }

    glm::vec3& IHitBox::size()
    {
        return _M_size;
    }

    // Point hitbox implementation

    PointHB::PointHB(const glm::vec3& position)
    {
        _M_position = position;
        _M_rotate = {0.f, 0.f, 0.f};
        _M_size = {0.f, 0.f, 0.f};
        _M_type = Engine::HitBoxType::POINT;
    }

    PointHB::PointHB(const PointHB& point) = default;
    PointHB& PointHB::operator=(const PointHB& point) = default;

    float PointHB::distance_to(const IHitBox& hitbox)
    {
        return distance_function_array[0][static_cast<int>(hitbox.type())](*this, hitbox);
    }

    // Box hitbox
    BoxHB::BoxHB(const glm::vec3& position, const glm::vec3& size, const glm::vec3& rotation)
    {
        _M_position = position;
        _M_size = size;
        _M_rotate = rotation;
        _M_type = HitBoxType::BOX;
    }

    BoxHB::BoxHB(const BoxHB&) = default;
    BoxHB& BoxHB::operator=(const BoxHB& box) = default;
    float BoxHB::distance_to(const IHitBox& hitbox)
    {
        return 0;
    }

    // Sphere hitbox
    SphereHB::SphereHB(const glm::vec3& position, const float& radius)
    {
        _M_position = position;
        _M_size = {radius, radius, radius};
        _M_type = Engine::HitBoxType::SPHERE;
    }

    SphereHB::SphereHB(const SphereHB&) = default;
    SphereHB& SphereHB::operator=(const SphereHB& box) = default;
    float SphereHB::distance_to(const IHitBox& hitbox)
    {
        return 0;
    }

    // Cylinder hitbox
    CylinderHB::CylinderHB(const glm::vec3& position, const float& radius, const float& height)
    {
        _M_position = position;
        _M_size = {radius, height, radius};
        _M_type = HitBoxType::CYLINDER;
    }

    CylinderHB::CylinderHB(const CylinderHB&) = default;
    CylinderHB& CylinderHB::operator=(const CylinderHB&) = default;
    float CylinderHB::distance_to(const IHitBox& hitbox)
    {
        return 0;
    }


}// namespace Engine
