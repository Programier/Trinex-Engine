#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/camera.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/window.hpp>
#include <functional>
#include <glm/ext.hpp>

namespace Engine
{
    implement_class(Camera, "Engine");
    implement_initialize_class(Camera)
    {}

    Camera::Camera(Vector3D position, float viewing_angle)
    {
        _M_viewing_angle = viewing_angle;
        Window* window   = engine_instance->window();
        if (window)
        {
            _M_aspect = window->width() / window->height();
        }
        transform.position = position;
        update_projection_matrix();
    }

    Camera& Camera::update_projection_matrix()
    {
        _M_projection = glm::perspective(_M_viewing_angle, _M_aspect, _M_min_render_distance, _M_max_render_distance);
        return *this;
    }

    const Matrix4f& Camera::projection()
    {
        return _M_projection;
    }

    Matrix4f Camera::projection(const glm::vec2& size) const
    {
        float temp_aspect = size.x / size.y;
        return glm::perspective(_M_viewing_angle, temp_aspect, _M_min_render_distance, _M_max_render_distance);
    }

    const Matrix4f& Camera::view()
    {
        return _M_view;
    }

    const Matrix4f& Camera::projview()
    {
        return _M_projview;
    }

    Camera& Camera::max_render_distance(float distance)
    {
        _M_max_render_distance = distance;
        return update_projection_matrix();
    }

    Camera& Camera::min_render_distance(float distance)
    {
        _M_min_render_distance = distance;
        return update_projection_matrix();
    }

    Distance& Camera::max_render_distance()
    {
        return _M_max_render_distance;
    }

    Distance& Camera::min_render_distance()
    {
        return _M_min_render_distance;
    }

    const Distance& Camera::max_render_distance() const
    {
        return _M_max_render_distance;
    }

    const Distance& Camera::min_render_distance() const
    {
        return _M_min_render_distance;
    }

    Camera& Camera::viewing_angle(float angle)
    {
        _M_viewing_angle = angle;
        return update_projection_matrix();
    }

    Engine::EulerAngle1D& Camera::viewing_angle()
    {
        return _M_viewing_angle;
    }

    const Engine::EulerAngle1D& Camera::viewing_angle() const
    {
        return _M_viewing_angle;
    }

    float Camera::aspect() const
    {
        return _M_aspect;
    }

    Camera& Camera::aspect(float value)
    {
        _M_aspect = value;
        return update_projection_matrix();
    }

    Camera& Camera::update(float dt)
    {
        Super::update(dt);
        auto front_vector   = transform.forward_vector();
        const Vector3D& pos = transform.position;

        _M_view     = glm::lookAt(pos, pos + front_vector, transform.up_vector());
        _M_projview = _M_projection * _M_view;
        return *this;
    }

    bool Camera::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
            return false;

        (*archive) & _M_viewing_angle;
        (*archive) & _M_max_render_distance;
        (*archive) & _M_min_render_distance;

        return static_cast<bool>(*archive);
    }

    Camera::~Camera()
    {}
}// namespace Engine
