#include <Graphics/camera.hpp>
#include <engine.hpp>
#include <functional>
#include <glm/ext.hpp>


namespace Engine
{
    Camera::Camera(glm::vec3 position, float viewingAngle, const bool& invert_z_vector) : _M_viewingAngle(viewingAngle)
    {
        _M_z_vector_mult = invert_z_vector ? -1.f : 1.f;
        move(position, false);
    }

    glm::mat4 Camera::projection(Window& window)
    {
        float aspect = (float) window.width() / (float) window.height();
        return glm::perspective(_M_viewingAngle, aspect, _M_minRenderDistance, _M_maxRenderDistance);
    }

    glm::mat4 Camera::projection(const glm::vec2& size)
    {
        return glm::perspective(_M_viewingAngle, size.x / size.y, _M_minRenderDistance, _M_maxRenderDistance);
    }

    glm::mat4 Camera::view()
    {
        return glm::lookAt(_M_position.get(), _M_position.get() + _M_z_vector_mult * _M_front.get(), _M_up.get());
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

    float Camera::max_render_distance()
    {
        return _M_maxRenderDistance;
    }

    float Camera::min_render_distance()
    {
        return _M_minRenderDistance;
    }

    Camera& Camera::viewing_angle(float angle)
    {
        _M_viewingAngle = angle;
        return *this;
    }

    float Camera::viewing_angle()
    {
        return _M_viewingAngle;
    }
}// namespace Engine