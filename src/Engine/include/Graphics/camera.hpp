#pragma once

#include <Graphics/basic_object.hpp>
#include <Window/window.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Engine
{

    class Camera : public BasicObject<Translate, Rotate>
    {
    private:
        float _M_z_vector_mult;
        float _M_viewingAngle;
        float _M_maxRenderDistance = 1000.0f, _M_minRenderDistance = 0.1f;

    public:
        Camera(glm::vec3 position, float fov, const bool& invert_z_vector = true);
        float max_render_distance();
        float min_render_distance();
        Camera& max_render_distance(float distance);
        Camera& min_render_distance(float distance);
        Camera& viewing_angle(float angle);
        float viewing_angle();
        glm::mat4 projection(Window& window);
        glm::mat4 view();
    };

}// namespace Engine
