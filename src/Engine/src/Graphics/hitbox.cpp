#include <Graphics/hitbox.hpp>
#include <glm/ext.hpp>
#include <vector>

#define in_range(value, min, max) (value >= min && value <= max)
#define THROW throw std::runtime_error(std::string("Not implemented method ") + __FUNCTION__)


static const std::vector<float> cube_mesh = {
        -1.f, -1.f, -1.f, -1.f, 1.f, -1.f, -1.f, -1.f, -1.f, 1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, 1.f,
        -1.f, 1.f,  -1.f, -1.f, 1.f, 1.f,  -1.f, 1.f,  -1.f, 1.f, 1.f,  -1.f, 1.f,  -1.f, -1.f, 1.f,  1.f,  -1.f,
        -1.f, -1.f, 1.f,  -1.f, 1.f, 1.f,  -1.f, 1.f,  1.f,  1.f, 1.f,  1.f,  -1.f, -1.f, 1.f,  1.f,  -1.f, 1.f,
        1.f,  -1.f, 1.f,  1.f,  1.f, 1.f,  1.f,  -1.f, 1.f,  1.f, -1.f, -1.f, 1.f,  1.f,  1.f,  1.f,  1.f,  -1.f};
static const std::vector<float> sphere;


//      DISTANCE FUNCTIONS

static glm::vec3 rotate_point(const glm::vec3& p, glm::quat r)
{
    return p * glm::inverse(r);
}

static glm::vec3 min_distance_from_point_to_box_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2,
                                                      bool* inside_result = nullptr)
{
    auto rotation = point1.quaternion();
    auto point = rotate_point(point1.position(), rotation);

    if (inside_result)
        *inside_result = false;
    auto cube = point2.position();
    auto& size = point2.size();

    auto min = cube - size;
    auto max = cube + size;

    auto vector = point - cube;

    int count = 0;
    float max_diff = 0;
    int s_index = 0;
    for (int i = 0; i < 3; i++)
    {
        if (glm::abs(vector[i]) <= size[i])
        {
            count++;
            if (max_diff < glm::abs(vector[i]))
            {
                max_diff = glm::abs(vector[i]);
                s_index = i;
            }
            vector[i] += cube[i];
        }
        else
            vector[i] = vector[i] < 0 ? min[i] : max[i];
    }

    if (count == 3)
    {
        glm::vec3 tmp = {0.f, 0.f, 0.f};
        tmp[s_index] = (size[s_index] - max_diff) * (vector[s_index] - cube[s_index] > 0 ? 1.f : -1.f);
        vector += tmp;
        if (inside_result)
            *inside_result = true;
    }

    return rotate_point(vector, glm::inverse(rotation));
}

static glm::vec3 min_distance_from_point_to_cylinder_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2,
                                                           bool* inside_result = nullptr)
{
    auto rotation = point2.quaternion();
    auto point = rotate_point(point1.position(), rotation);
    auto& size = point2.size();
    auto cylinder = point2.position();
    auto vector = point - point2.position();
    if (inside_result)
        *inside_result = false;
    float K = vector[2] == 0 ? 0 : vector[0] / vector[2];
    float X = size.r * glm::sqrt(1.0f / (1.0f + K * K));
    if (glm::abs(vector[1]) <= size[1])
    {
        vector[0] = cylinder.x + X;
        vector[1] += cylinder.y;
        vector[2] = cylinder.z + X * K;
        if (inside_result && vector[0] * vector[0] + vector[2] * vector[2] <= size.r * size.r)
            *inside_result = true;
    }
    else if ((vector[0] * vector[0] + vector[2] * vector[2]) <= (size.r * size.r))
    {
        if (inside_result && glm::abs(vector[1]) <= size[1])
            *inside_result = true;
        vector[0] += cylinder[0];
        vector[1] = vector[1] < 0 ? cylinder.y - size.y : cylinder.y + size.y;
        vector[2] += cylinder[2];
    }
    else
    {
        vector[0] = cylinder.x + X;
        vector[1] = vector[1] > 0 ? cylinder.y + size.y : cylinder.y - size.y;
        vector[2] = cylinder.z + X * K;
    }
    return vector;
}

// From point distance
static float from_point_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::distance(point1.position(), point2.position());
}

static float from_point_to_box(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    bool inside = false;
    auto point_2 = min_distance_from_point_to_box_point(point1, point2, &inside);
    return inside ? -glm::distance(point1.position(), point_2) : glm::distance(point1.position(), point_2);
}

static float from_point_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::distance(point1.position(), point2.position()) - point2.size().r;
}

static float from_point_to_cylinder(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{

    bool inside = false;
    auto result = min_distance_from_point_to_cylinder_point(point1, point2, &inside);
    return inside ? -glm::distance(point1.position(), result) : glm::distance(point1.position(), result);
}

// From box distance
static float from_box_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_box(point2, point1);
}

static float from_box_to_box(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    bool inside_1 = false, inside_2 = false;
    auto point_1 = min_distance_from_point_to_box_point(point1, point2, &inside_1);
    auto point_2 = min_distance_from_point_to_box_point(Engine::PointHB(point_1), point1, &inside_2);
    return (inside_1 || inside_2) ? -glm::distance(point_1, point_2) : glm::distance(point_1, point_2);
}

static float from_box_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_box(point2, point1) - point2.size().r;
}

static float from_box_to_cylinder(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    bool inside_1 = false, inside_2 = false;
    auto point_1 = min_distance_from_point_to_cylinder_point(point1, point2, &inside_1);
    auto point_2 = min_distance_from_point_to_box_point(Engine::PointHB(point_1), point1, &inside_2);
    return (inside_1 || inside_2) ? -glm::distance(point_1, point_2) : glm::distance(point_1, point_2);
}

// From sphere distance
static float from_sphere_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_sphere(point2, point1);
}

static float from_sphere_to_box(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return Engine::PointHB(point1.position()).distance_to(point2) - point1.size().x;
}

static float from_sphere_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::distance(point1.position(), point2.position()) - point1.size().r - point2.size().r;
}

static float from_sphere_to_cylinder(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_cylinder(point1, point2) - point1.size().r;
}

// From cylinder distance
static float from_cylinder_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_cylinder(point2, point1);
}

static float from_cylinder_to_box(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_box_to_cylinder(point2, point1);
}

static float from_cylinder_to_sphere(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return from_point_to_cylinder(point2, point1) - point2.size().x;
}

static float from_cylinder_to_cylinder(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    bool inside_1 = false, inside_2 = false;
    auto point_1 = min_distance_from_point_to_cylinder_point(point1, point2, &inside_1);
    auto point_2 = min_distance_from_point_to_cylinder_point(Engine::PointHB(point_1), point1, &inside_2);
    return (inside_1 || inside_2) ? -glm::distance(point_1, point_2) : glm::distance(point_1, point_2);
}

static float (*distance_function_array[HITBOX_TYPES_COUNT][HITBOX_TYPES_COUNT])(const Engine::IHitBox&,
                                                                                const Engine::IHitBox&) = {
        {from_point_to_point, from_point_to_box, from_point_to_sphere, from_point_to_cylinder},
        {from_box_to_point, from_box_to_box, from_box_to_sphere, from_box_to_cylinder},
        {from_sphere_to_point, from_sphere_to_box, from_sphere_to_sphere, from_sphere_to_cylinder},
        {from_cylinder_to_point, from_cylinder_to_box, from_cylinder_to_sphere, from_cylinder_to_cylinder}};


namespace Engine
{
    const HitBoxType& IHitBox::type() const
    {
        return _M_type;
    }

    const glm::vec3& IHitBox::size() const
    {
        return _M_size;
    }

    glm::vec3& IHitBox::size()
    {
        return _M_size;
    }

    float IHitBox::distance_to(const IHitBox& hitbox)
    {
        return distance_function_array[static_cast<int>(_M_type)][static_cast<int>(hitbox.type())](*this, hitbox);
    }

    Line& IHitBox::lines()
    {
        return _M_lines;
    }

    // Point hitbox implementation

    PointHB::PointHB(const glm::vec3& position)
    {
        move(position, false);
        _M_size = {0.f, 0.f, 0.f};
        _M_type = Engine::HitBoxType::POINT;
    }

    PointHB::PointHB(const PointHB& point) = default;
    PointHB& PointHB::operator=(const PointHB& point) = default;

    // Box hitbox
    BoxHB::BoxHB(const glm::vec3& position, const glm::vec3& size, const glm::vec3& rotation)
    {
        move(position, false);
        _M_size = size;
        rotate(rotation, false);
        _M_type = HitBoxType::BOX;

        _M_lines.data() = cube_mesh;
        _M_lines.attributes({3}).vertices_count(24).update_buffers();
    }

    BoxHB::BoxHB(const BoxHB&) = default;
    BoxHB& BoxHB::operator=(const BoxHB& box) = default;

    // Sphere hitbox
    SphereHB::SphereHB(const glm::vec3& position, const float& radius)
    {
        _M_position = position;
        _M_size = {radius, radius, radius};
        _M_type = Engine::HitBoxType::SPHERE;
    }

    SphereHB::SphereHB(const SphereHB&) = default;
    SphereHB& SphereHB::operator=(const SphereHB& box) = default;

    // Cylinder hitbox
    CylinderHB::CylinderHB(const glm::vec3& position, const float& radius, const float& height)
    {
        _M_position = position;
        _M_size = {radius, height, radius};
        _M_type = HitBoxType::CYLINDER;
    }

    CylinderHB::CylinderHB(const CylinderHB&) = default;
    CylinderHB& CylinderHB::operator=(const CylinderHB&) = default;


}// namespace Engine