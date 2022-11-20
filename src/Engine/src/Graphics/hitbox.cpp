#include <Core/logger.hpp>
#include <Graphics/hitbox.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/scene.hpp>
#include <Graphics/shader_system.hpp>
#include <api_funcs.hpp>
#include <iostream>


#define ex_min(a, b, c) glm::min(glm::min(a, b), c)
#define ex_max(a, b, c) glm::max(glm::max(a, b), c)

namespace Engine
{
    HitBox::~HitBox() = default;

    BoxHB::BoxHB()
    {
        _M_center = _M_half_sizes = min = max = Constants::zero_vector;
    }

    BoxHB::BoxHB(const BoxHB&) = default;
    BoxHB::BoxHB(BoxHB&&) = default;
    BoxHB& BoxHB::operator=(const BoxHB&) = default;
    BoxHB& BoxHB::operator=(BoxHB&&) = default;

    BoxHB::BoxHB(const AABB_3D& box)
    {
        aabb(box);
    }

    BoxHB::BoxHB(const Size3D& _min, const Size3D& _max)
    {
        aabb(_min, _max);
    }

    const AABB_3D& BoxHB::aabb() const
    {
        return *this;
    }

    BoxHB& BoxHB::aabb(const AABB_3D& box)
    {
        return aabb(box.min, box.max);
    }

    BoxHB& BoxHB::aabb(const BoxHB& box)
    {
        *this = box;
        return *this;
    }

    BoxHB& BoxHB::aabb(const Size3D& _min, const Size3D& _max)
    {
        min = glm::min(_min, _max);
        max = glm::max(_min, _max);
        _M_center = (max + min) / 2.f;
        _M_half_sizes = max - _M_center;
        return *this;
    }
    bool BoxHB::is_on_or_forward_plan(const Plane& plane) const
    {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = _M_half_sizes.x * std::abs(plane.normal.x) + _M_half_sizes.y * std::abs(plane.normal.y) +
                        _M_half_sizes.z * std::abs(plane.normal.z);

        return -r <= plane.get_signed_distance_to_plan(_M_center);
    }

    bool BoxHB::is_in_frustum(const Frustum& frustum) const
    {
        return (is_on_or_forward_plan(frustum.left) && is_on_or_forward_plan(frustum.right) &&
                is_on_or_forward_plan(frustum.top) && is_on_or_forward_plan(frustum.bottom) &&
                is_on_or_forward_plan(frustum.near) && is_on_or_forward_plan(frustum.far));
    }


    BoxHB BoxHB::apply_model(const glm::mat4& model) const
    {

        //        BoxHB result;

        //        //        // Calculate new center position

        //        result._M_center = model * Vector4D(_M_center, 1.f);

        //        result.max = model * Vector4D(max, 1.f);

        //        result.max = glm::max(Point3D(model * Vector4D(-max, 1.f)), result.max);
        //        result.max = glm::max(Point3D(model * Vector4D(max.x, max.y, min.z, 1.f)), result.max);

        //        result._M_half_sizes = result.max - result._M_center;
        //        result.min = result._M_center - result._M_half_sizes;


        Point3D points[8];

        Vector4D vector(max, 1.f);
        points[0] = model * vector;

        vector.z = min.z;
        points[1] = model * vector;

        vector.z = max.z;
        vector.y = min.y;
        points[2] = model * vector;

        vector.z = min.z;
        points[3] = model * vector;

        vector.z = max.z;
        vector.y = max.y;
        vector.x = min.x;
        points[4] = model * vector;

        vector.z = min.z;
        points[5] = model * vector;

        vector.z = max.z;
        vector.y = min.y;
        points[6] = model * vector;

        vector.z = min.z;
        points[7] = model * vector;

        AABB_3D result(points[0], points[0]);

        for (auto& point : points)
        {
            result.max = glm::max(result.max, point);
            result.min = glm::min(result.min, point);
        }


        return result;
    }

    bool BoxHB::is_in_frustum(const Frustum& frustum, const glm::mat4& model) const
    {
        return apply_model(model).is_in_frustum(frustum);
    }


    static bool _M_mesh_inited = false;
    static Mesh<float> _M_mesh;


    void BoxHB::render(const glm::mat4& model) const
    {
        Scene* scene = Scene::get_active_scene();
        if (!scene || !scene->active_camera().camera)
            throw std::runtime_error("No active scene or camera found!");
        const Scene::ActiveCamera& active_camera = scene->active_camera();

        if (!_M_mesh_inited)
        {
            _M_mesh.attributes = {{3, BufferValueType::FLOAT}};
            _M_mesh.data = {-1, -1, -1,// 0
                            -1, -1, 1, // 1
                            -1, 1,  -1,// 2
                            -1, 1,  1, // 3
                            1,  -1, -1,// 4
                            1,  -1, 1, // 5
                            1,  1,  -1,// 6
                            1,  1,  1};// 7

            _M_mesh.indexes = {0, 1,//
                               0, 2,//
                               0, 4,//
                               1, 3,//
                               1, 5,//
                               5, 7,//
                               5, 4,//
                               4, 6,//
                               6, 7,//
                               6, 2,//
                               3, 2,//
                               3, 7};
            _M_mesh.vertices = _M_mesh.indexes.size();

            _M_mesh.mode = DrawMode::STATIC_DRAW;
            _M_mesh.gen();


            _M_mesh.set_data();
            _M_mesh.update_atributes().update_indexes();
        }

        float _M_line_offset = 0.00001f;
        float _M_line_width = get_current_line_rendering_width();
        set_line_rendering_width(3.f);
        _M_mesh.data = {min.x - _M_line_offset, min.y - _M_line_offset, min.z - _M_line_offset,//
                        min.x - _M_line_offset, min.y - _M_line_offset, max.z + _M_line_offset,//
                        min.x - _M_line_offset, max.y + _M_line_offset, min.z - _M_line_offset,//
                        min.x - _M_line_offset, max.y + _M_line_offset, max.z + _M_line_offset,//
                        max.x + _M_line_offset, min.y - _M_line_offset, min.z - _M_line_offset,//
                        max.x + _M_line_offset, min.y - _M_line_offset, max.z + _M_line_offset,//
                        max.x + _M_line_offset, max.y + _M_line_offset, min.z - _M_line_offset,//
                        max.x + _M_line_offset, max.y + _M_line_offset, max.z + _M_line_offset};

        _M_mesh.update_data(0, sizeof(float) * 24);
        namespace sh = ShaderSystem::Line;
        sh::shader.use().set(sh::model, model).set(sh::color, Color::Red).set(sh::projview, active_camera.projview);
        _M_mesh.draw(Primitive::LINE);

        set_line_rendering_width(_M_line_width);
    }

    BoxHB BoxHB::max_box(const AABB_3D& box) const
    {
        AABB_3D result = *this;
        result.max = glm::max(result.max, glm::max(box.min, box.max));
        result.min = glm::min(result.min, glm::min(box.min, box.max));
        return result;
    }

    const Point3D& BoxHB::center() const
    {
        return _M_center;
    }

    const Size3D& BoxHB::half_size() const
    {
        return _M_half_sizes;
    }

    bool BoxHB::contains(const AABB_3D& box) const
    {
        return contains(BoxHB(box));
    }

#define contains_check(coord)                                                                                               \
    if (_M_half_sizes.coord < box._M_half_sizes.coord)                                                                      \
        return false;                                                                                                       \
    if (center_offset.coord + box._M_half_sizes.coord > _M_half_sizes.coord)                                                \
    return false

    bool BoxHB::contains(const BoxHB& box) const
    {
        auto center_offset = glm::abs(box._M_center - _M_center);

        contains_check(x);
        contains_check(y);
        contains_check(z);

        return true;
    }

    bool BoxHB::inside(const AABB_3D& box) const
    {
        return BoxHB(box).contains(*this);
    }

    bool BoxHB::inside(const BoxHB& box) const
    {
        return BoxHB(box).contains(*this);
    }

    Vector2D BoxHB::intersect(const Ray& ray) const
    {
        Vector3D t_min = (min - ray.origin()) / ray.direction();
        Vector3D t_max = (max - ray.origin()) / ray.direction();

        Vector3D t1 = glm::min(t_min, t_max);
        Vector3D t2 = glm::max(t_min, t_max);

        float near = glm::max(glm::max(t1.x, t1.y), t1.z);
        float far = glm::min(glm::min(t2.x, t2.y), t2.z);
        return Vector2D(near, far);
    }
}// namespace Engine
