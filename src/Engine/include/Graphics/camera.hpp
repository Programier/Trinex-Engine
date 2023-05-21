#pragma once

#include <Core/actor.hpp>

#include <Core/export.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/transform_component.hpp>
#include <Window/window.hpp>


namespace Engine
{

    class ENGINE_EXPORT Camera : public Actor
    {
    private:
        EulerAngle1D _M_viewing_angle;
        Distance _M_max_render_distance = 100.0f, _M_min_render_distance = 0.5f;
        mutable float _M_aspect = 0.f;

        Matrix4f _M_projection, _M_view, _M_projview;

        Camera& update_projection_matrix();
    public:

        delete_copy_constructors(Camera);
        Camera(Point3D position = {0.f, 0.f, 0.f}, float fov = glm::radians(90.f));
        Distance& max_render_distance();
        Distance& min_render_distance();
        const Distance& max_render_distance() const;
        const Distance& min_render_distance() const;
        Camera& max_render_distance(float distance);
        Camera& min_render_distance(float distance);
        Camera& viewing_angle(float angle);
        EulerAngle1D& viewing_angle();
        const EulerAngle1D& viewing_angle() const;
        const Matrix4f& projection();
        Matrix4f projection(const Size2D& size) const;
        const Matrix4f& view();
        const Matrix4f& projview();
        float aspect() const;
        Camera& aspect(float value);
        Camera& update() override;
        ~Camera();
    };

}// namespace Engine
