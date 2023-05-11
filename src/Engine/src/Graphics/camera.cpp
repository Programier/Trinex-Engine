#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/camera.hpp>
#include <functional>
#include <glm/ext.hpp>

namespace Engine
{

    static void on_camera_event(TransformComponents::TranslateRotate* value)
    {
        Camera* camera = reinterpret_cast<Camera*>(value);
        camera->need_update(true);
    }

    bool Camera::need_update() const
    {
        return _M_need_update;
    }

    Camera& Camera::need_update(bool flag)
    {
        _M_need_update = flag;
        return *this;
    }

    register_class(Engine::Camera, Engine::Object);
    Camera::Camera(glm::vec3 position, float viewing_angle)
    {
        _M_viewingAngle = viewing_angle;
        move(position, false);
        info_log("Created camera");
        _M_translate_fields.on_translate.push(on_camera_event);
        _M_rotate_fields.on_rotate.push(on_camera_event);
    }

    Camera& Camera::update_matrices()
    {
        if (_M_need_update)
        {
            _M_projection  = glm::perspective(_M_viewingAngle, _M_aspect, _M_minRenderDistance, _M_maxRenderDistance);
            _M_view        = glm::lookAt(position(), position() + front_vector(), up_vector());
            _M_projview    = _M_projection * _M_view;
            _M_need_update = false;
        }
        return *this;
    }

    const Matrix4f& Camera::projection()
    {
        update_matrices();
        return _M_projection;
    }

    Matrix4f Camera::projection(const glm::vec2& size) const
    {
        float temp_aspect = size.x / size.y;
        return glm::perspective(_M_viewingAngle, temp_aspect, _M_minRenderDistance, _M_maxRenderDistance);
    }

    const Matrix4f& Camera::view()
    {
        update_matrices();
        return _M_view;
    }

    const Matrix4f& Camera::projview()
    {
        update_matrices();
        return _M_projview;
    }

    Camera& Camera::max_render_distance(float distance)
    {
        _M_maxRenderDistance = distance;
        _M_need_update       = true;
        return *this;
    }

    Camera& Camera::min_render_distance(float distance)
    {
        _M_minRenderDistance = distance;
        _M_need_update       = true;
        return *this;
    }

    Distance& Camera::max_render_distance()
    {
        return _M_maxRenderDistance;
    }

    Distance& Camera::min_render_distance()
    {
        return _M_minRenderDistance;
    }

    const Distance& Camera::max_render_distance() const
    {
        return _M_maxRenderDistance;
    }

    const Distance& Camera::min_render_distance() const
    {
        return _M_minRenderDistance;
    }

    Camera& Camera::viewing_angle(float angle)
    {
        _M_viewingAngle = angle;
        _M_need_update  = true;
        return *this;
    }

    Engine::EulerAngle1D& Camera::viewing_angle()
    {
        return _M_viewingAngle;
    }

    const Engine::EulerAngle1D& Camera::viewing_angle() const
    {
        return _M_viewingAngle;
    }

    Vector3D Camera::front_vector() const
    {
        return -TransformComponent::front_vector();
    }

    float Camera::aspect() const
    {
        return _M_aspect;
    }

    Camera& Camera::aspect(float value)
    {
        _M_aspect      = value;
        _M_need_update = true;
        return *this;
    }

    Camera::~Camera()
    {}
}// namespace Engine
