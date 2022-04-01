#include <Graphics/camera.hpp>

#include <glm/ext.hpp>

namespace Engine
{
    Camera::Camera(glm::vec3 position, float viewingAngle)
        : _M_position(position), _M_viewingAngle(viewingAngle), _M_rotation(1.0f)
    {

        _M_x_rotation = 0;
        _M_y_rotation = 0;
        _M_z_rotation = 0;
        updateVectors();
    }

    void Camera::updateVectors()
    {
        _M_front = glm::vec3(_M_rotation * glm::vec4(0, 0, -1, 1));
        _M_right = glm::vec3(_M_rotation * glm::vec4(1, 0, 0, 1));
        _M_up = glm::vec3(_M_rotation * glm::vec4(0, 1, 0, 1));
    }

    Camera& Camera::rotate(float x, float y, float z)
    {
        _M_x_rotation += x;
        _M_y_rotation += y;
        _M_z_rotation += z;
        _M_rotation = glm::mat4(1.0f);
        _M_rotation = glm::rotate(_M_rotation, _M_z_rotation, glm::vec3(0, 0, 1));
        _M_rotation = glm::rotate(_M_rotation, _M_y_rotation, glm::vec3(0, 1, 0));
        _M_rotation = glm::rotate(_M_rotation, _M_x_rotation, glm::vec3(1, 0, 0));

        updateVectors();
        return *this;
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

    glm::vec3 Camera::rotation()
    {
        return glm::vec3(_M_x_rotation, _M_y_rotation, _M_z_rotation);
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

    Camera& Camera::rotate(const glm::vec3& _M_rotation)
    {
        return rotate(_M_rotation.x, _M_rotation.y, _M_rotation.z);
    }

}// namespace Engine
