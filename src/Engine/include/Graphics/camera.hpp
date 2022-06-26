#pragma once

#include <Graphics/basic_object.hpp>
#include <Window/window.hpp>


namespace Engine
{

    class Camera : public BasicObject<Translate, Rotate>
    {
    private:
        float _M_z_vector_mult;
        EulerAngle1D _M_viewingAngle;
        Distance _M_maxRenderDistance = 100.0f, _M_minRenderDistance = 0.5f;

    public:
        Camera(Point3D position, float fov, const bool& invert_z_vector = true);
        Distance max_render_distance();
        Distance min_render_distance();
        Camera& max_render_distance(float distance);
        Camera& min_render_distance(float distance);
        Camera& viewing_angle(float angle);
        EulerAngle1D viewing_angle();
        glm::mat4 projection(Window& window);
        glm::mat4 projection(const Size2D& size);
        glm::mat4 view();
    };

}// namespace Engine
