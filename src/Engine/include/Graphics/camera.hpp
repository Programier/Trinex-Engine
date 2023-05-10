#pragma once

#include <Core/export.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/transform_component.hpp>
#include <Window/window.hpp>


namespace Engine
{

    class ENGINE_EXPORT Camera : public TransformComponents::TranslateRotate
    {
    private:
        EulerAngle1D _M_viewingAngle;
        Distance _M_maxRenderDistance = 100.0f, _M_minRenderDistance = 0.5f;
        mutable float _M_aspect = 0.f;
        bool _M_need_update     = true;

        Matrix4f _M_projection, _M_view, _M_projview;

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
        Camera& update_matrices();
        Vector3D front_vector() const;
        bool need_update() const;
        Camera& need_update(bool flag);
        ~Camera();
    };

}// namespace Engine
