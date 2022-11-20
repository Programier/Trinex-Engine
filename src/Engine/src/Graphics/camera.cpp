#include <Core/engine.hpp>
#include <Graphics/camera.hpp>
#include <functional>
#include <glm/ext.hpp>

namespace Engine
{
    Camera::Camera(glm::vec3 position, float viewingAngle, const std::wstring& name)
        : _M_viewingAngle(viewingAngle), name(name)
    {
        move(position, false);
    }

    glm::mat4 Camera::projection()
    {
        //_M_aspect = (float) Window::width() / (float) Window::height();
        return glm::perspective(_M_viewingAngle, _M_aspect, _M_minRenderDistance, _M_maxRenderDistance);
    }

    glm::mat4 Camera::projection(const glm::vec2& size)
    {
        float temp_aspect = size.x / size.y;
        return glm::perspective(_M_viewingAngle, temp_aspect, _M_minRenderDistance, _M_maxRenderDistance);
    }

    glm::mat4 Camera::view()
    {
        auto front = front_vector();
        auto up = up_vector(false);
        return glm::lookAt(_M_position.get(), _M_position.get() + front, up);
    }


    Camera& Camera::max_render_distance(float distance)
    {
        _M_maxRenderDistance = distance;
        return *this;
    }

    Camera& Camera::min_render_distance(float distance)
    {
        _M_minRenderDistance = distance;
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

    Vector3D Camera::front_vector(bool update) const
    {
        return -Rotate::front_vector(update);
    }

    float Camera::aspect() const
    {
        return _M_aspect;
    }

    Camera& Camera::aspect(float value)
    {
        _M_aspect = value;
        return *this;
    }
}// namespace Engine
