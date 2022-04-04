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
    for (int i = 0; i < 3; i++)
    {
        if (glm::abs(vector[i]) <= size[i])
            index += i;
        else
            vector[i] = vector[i] < 0 ? min[i] : max[i];
    }
    return (index != 0) ? glm::distance(point, cube) - size[3 - index] : glm::distance(point, vector);
}

static float from_point_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::abs(glm::distance(point1.position(), point2.position())) - point2.size().r;
}

static float from_point_to_cylinder(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return 0;
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
    }

    PointHB::PointHB(const PointHB& point) = default;
    PointHB& PointHB::operator=(const PointHB& point) = default;

    float PointHB::distance_to(const IHitBox& hitbox)
    {
        return distance_function_array[0][static_cast<int>(hitbox.type())](*this, hitbox);
    }

    // Box hitbox
    BoxHB::BoxHB(const glm::vec3& position, const glm::vec3& size)
    {
        _M_position = position;
        _M_size = size;
        _M_type = HitBoxType::BOX;

        _M_lines.push_line({position.x - size.x, position.y - size.y, position.z - size.z},
                           {position.x + size.x, position.y - size.y, position.z - size.z});
        _M_lines.push_line({position.x - size.x, position.y - size.y, position.z - size.z},
                           {position.x - size.x, position.y + size.y, position.z - size.z});
        _M_lines.push_line({position.x + size.x, position.y - size.y, position.z - size.z},
                           {position.x + size.x, position.y + size.y, position.z - size.z});
        _M_lines.push_line({position.x + size.x, position.y + size.y, position.z - size.z},
                           {position.x - size.x, position.y + size.y, position.z - size.z});

        _M_lines.push_line({position.x - size.x, position.y - size.y, position.z + size.z},
                           {position.x + size.x, position.y - size.y, position.z + size.z});
        _M_lines.push_line({position.x - size.x, position.y - size.y, position.z + size.z},
                           {position.x - size.x, position.y + size.y, position.z + size.z});
        _M_lines.push_line({position.x + size.x, position.y - size.y, position.z + size.z},
                           {position.x + size.x, position.y + size.y, position.z + size.z});
        _M_lines.push_line({position.x + size.x, position.y + size.y, position.z + size.z},
                           {position.x - size.x, position.y + size.y, position.z + size.z});

        _M_lines.push_line({position.x + size.x, position.y - size.y, position.z - size.z},
                           {position.x + size.x, position.y - size.y, position.z + size.z});
        _M_lines.push_line({position.x + size.x, position.y + size.y, position.z - size.z},
                           {position.x + size.x, position.y + size.y, position.z + size.z});

        _M_lines.push_line({position.x - size.x, position.y - size.y, position.z - size.z},
                           {position.x - size.x, position.y - size.y, position.z + size.z});
        _M_lines.push_line({position.x - size.x, position.y + size.y, position.z - size.z},
                           {position.x - size.x, position.y + size.y, position.z + size.z});
    }

    BoxHB::BoxHB(const BoxHB&) = default;
    BoxHB& BoxHB::operator=(const BoxHB& box) = default;
    float BoxHB::distance_to(const IHitBox& hitbox)
    {
        return 0;
    }

    BoxHB& BoxHB::draw(const float& width)
    {
        _M_lines.line_width(width).draw();
        return *this;
    }


}// namespace Engine
