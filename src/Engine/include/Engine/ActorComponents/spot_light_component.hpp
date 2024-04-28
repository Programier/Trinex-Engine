#pragma once

#include <Engine/ActorComponents/point_light_component.hpp>

namespace Engine
{
    class ENGINE_EXPORT SpotLightComponentProxy : public PointLightComponentProxy
    {
        float m_angle;
        float m_cos_cutoff;

    public:
        float angle() const;
        float cos_cutoff() const;
        SpotLightComponentProxy& angle(float value);
        Vector3D direction() const;

        friend class SpotLightComponent;
    };

    class ENGINE_EXPORT SpotLightComponent : public PointLightComponent
    {
        declare_class(SpotLightComponent, PointLightComponent);

    private:
        float m_angle;

        SpotLightComponent& submit_spot_light_data();

    public:
        SpotLightComponent();

        float angle() const;
        SpotLightComponent& angle(float value);

        Vector3D direction() const;
        SpotLightComponentProxy* proxy() const;

        Type light_type() const override;
        SpotLightComponent& spawned() override;
        SpotLightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        SpotLightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
        ActorComponentProxy* create_proxy() override;
    };

}// namespace Engine
