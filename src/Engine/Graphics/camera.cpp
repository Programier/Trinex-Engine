#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/camera.hpp>
#include <glm/ext.hpp>

namespace Engine
{
    Camera::Camera(glm::vec3 position, float viewingAngle) : _M_position(position), _M_viewingAngle(viewingAngle)
    {
        _M_euler_angles = glm::vec3(0.f, 0.f, 0.f);
        _M_quaternion = glm::quat(_M_euler_angles);
        updateVectors();
    }

    void Camera::updateVectors()
    {
        _M_front = glm::normalize(_M_quaternion * glm::vec4(0.f, 0.f, -1.f, 1.0f));
        _M_up = glm::normalize(_M_quaternion * glm::vec4(0.f, 1.f, 0.f, 1.0f));
        _M_right = glm::normalize(_M_quaternion * glm::vec4(1.f, 0.f, 0.f, 1.0f));
    }

    Camera& Camera::rotate(float x, float y, float z, const bool& add_values)
    {
        return rotate({x, y, z}, add_values);
    }

    glm::mat4 Camera::projection(Window& window)
    {
        float aspect = (float) window.width() / (float) window.height();
        return glm::perspective(_M_viewingAngle, aspect, _M_minRenderDistance, _M_maxRenderDistance);
    }

    glm::mat4 Camera::view()
    {
        return glm::lookAt(_M_position, _M_position + _M_front, _M_up);
    }

    Camera& Camera::move(const float& right, const float& up, const float& forward)
    {
        _M_position += _M_front * forward;
        _M_position += _M_right * right;
        _M_position += _M_up * up;
        return *this;
    }

    Camera& Camera::move(const glm::vec3& move_vector)
    {
        return move(move_vector[0], move_vector[1], move_vector[2]);
    }

    Camera& Camera::move_along_axes(const float& right, const float& up, const float& forward, const glm::vec3& right_a,
                                    const glm::vec3& up_a, const glm::vec3& front_a)
    {
        _M_position += front_a * forward;
        _M_position += right_a * right;
        _M_position += up_a * up;
        return *this;
    }

    Camera& Camera::move_along_axes(const glm::vec3& move_vector, const glm::vec3& right_a, const glm::vec3& up_a,
                                    const glm::vec3& front_a)
    {
        return move_along_axes(move_vector[0], move_vector[1], move_vector[2], right_a, up_a, front_a);
    }

    glm::vec3 Camera::front_vector()
    {
        return _M_front;
    }

    glm::vec3 Camera::right_vector()
    {
        return _M_right;
    }

    glm::vec3 Camera::up_vector()
    {
        return _M_up;
    }

    glm::vec3 Camera::coords()
    {
        return _M_position;
    }

    Camera& Camera::coords(const glm::vec3& c)
    {
        _M_position = c;
        return *this;
    }

    const glm::quat& Camera::quaternion()
    {
        return _M_quaternion;
    }

    const glm::vec3& Camera::rotation()
    {
        return _M_euler_angles;
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

    Camera& Camera::rotate(const glm::vec3& _rotation, const bool& add_values)
    {
        if (add_values)
            _M_euler_angles += _rotation;
        else
            _M_euler_angles = _rotation;
        _M_quaternion = glm::quat(_M_euler_angles);
        updateVectors();
        return *this;
    }
}// namespace Engine
