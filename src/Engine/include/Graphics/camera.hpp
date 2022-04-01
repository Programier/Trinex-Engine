#pragma once

#include <Window/window.hpp>
#include <glm/glm.hpp>
namespace Engine
{

    class Camera
    {
    private:
        void updateVectors();
        glm::vec3 _M_front;
        glm::vec3 _M_up;
        glm::vec3 _M_right;

        glm::vec3 _M_position;
        float _M_viewingAngle, _M_x_rotation, _M_y_rotation, _M_z_rotation;
        glm::mat4 _M_rotation;
        float _M_maxRenderDistance = 1000.0f, _M_minRenderDistance = 0.1f;

    public:
        Camera(glm::vec3 position, float fov);
        float max_render_distance();
        float min_render_distance();
        Camera& max_render_distance(float distance);
        Camera& min_render_distance(float distance);
        Camera& viewing_angle(float angle);
        float viewing_angle();
        Camera& rotate(float x, float y, float z);
        Camera& rotate(const glm::vec3& rotation);
        glm::vec3 rotation();
        Camera& move(const float& right, const float& up, const float& forward);
        Camera& move(const glm::vec3& move_vector);
        Camera& move_along_axes(const float& right, const float& up, const float& forward, const glm::vec3& right_a,
                                const glm::vec3& up_a, const glm::vec3& front_a);
        Camera& move_along_axes(const glm::vec3& move_vector, const glm::vec3& right_a, const glm::vec3& up_a,
                                const glm::vec3& front_a);
        glm::vec3 front_vector();
        glm::vec3 right_vector();
        glm::vec3 up_vector();

        glm::mat4 projection(Window& window);
        glm::mat4 view();
        glm::vec3 coords();
        Camera& coords(const glm::vec3& coords);
    };

}// namespace Engine
