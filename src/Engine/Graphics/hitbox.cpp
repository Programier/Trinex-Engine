#include <Graphics/hitbox.hpp>
#include <glm/ext.hpp>
#include <vector>


/*

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
*/

// From point distance
static float from_point_to_point(const Engine::IHitBox& point1, const Engine::IHitBox& point2)
{
    return glm::abs(glm::distance(point1.position(), point2.position()));
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

    if (point.x <= max.x && point.x >= min.x && point.z <= max.z && point.z >= min.z)
        return glm::abs(glm::distance(point, cube)) - size.y;

    if (point.x <= max.x && point.x >= min.x && point.y <= max.y && point.y >= min.y)
        return glm::abs(glm::distance(point, cube)) - size.z;

    if (point.y <= max.y && point.y >= min.y && point.z <= max.z && point.z >= min.z)
        return glm::abs(glm::distance(point, cube)) - size.x;


    if (point.x > max.x && point.y > max.y)
        return glm::abs(glm::distance(point, {max.x, max.y, point.z}));

    if (point.x > max.x && point.z > max.z)
        return glm::abs(glm::distance(point, {max.x, point.y, max.z}));

    if (point.y > max.y && point.z > max.z)
        return glm::abs(glm::distance(point, {point.x, max.y, max.z}));


    if (point.x < min.x && point.y < min.y)
        return glm::abs(glm::distance(point, {min.x, min.y, point.z}));

    if (point.x < min.x && point.z < min.z)
        return glm::abs(glm::distance(point, {min.x, point.y, min.z}));

    if (point.y < min.y && point.z < min.z)
        return glm::abs(glm::distance(point, {point.x, min.y, min.z}));

    return 0;
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

    Point::Point(const glm::vec3& position)
    {
        _M_position = position;
        _M_rotate = {0.f, 0.f, 0.f};
        _M_size = {0.f, 0.f, 0.f};
    }

    Point::Point(const Point& point) = default;
    Point& Point::operator=(const Point& point) = default;

    float Point::distance_to(const IHitBox& hitbox)
    {
        return distance_function_array[0][static_cast<int>(hitbox.type())](*this, hitbox);
    }

}// namespace Engine
