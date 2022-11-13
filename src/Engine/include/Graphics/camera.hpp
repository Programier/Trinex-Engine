#pragma once

#include <Core/export.hpp>
#include <Graphics/basic_object.hpp>
#include <Window/window.hpp>


namespace Engine
{

    CLASS Camera : public BasicObject<Translate, Rotate>
    {
    private:
        EulerAngle1D _M_viewingAngle;
        Distance _M_maxRenderDistance = 100.0f, _M_minRenderDistance = 0.5f;
        float _M_aspect = 0.f;

    public:
        std::wstring name;
        Camera(Point3D position = {0.f, 0.f, 0.f}, float fov = glm::radians(90.f), const std::wstring& name = L"");
        Distance& max_render_distance();
        Distance& min_render_distance();
        const Distance& max_render_distance() const;
        const Distance& min_render_distance() const;
        Camera& max_render_distance(float distance);
        Camera& min_render_distance(float distance);
        Camera& viewing_angle(float angle);
        EulerAngle1D& viewing_angle();
        const EulerAngle1D& viewing_angle() const;
        glm::mat4 projection();
        glm::mat4 projection(const Size2D& size);
        glm::mat4 view();
        float aspect() const;
        Camera& aspect(float value);

        Vector3D front_vector(bool update = true) const override;
    };

}// namespace Engine
