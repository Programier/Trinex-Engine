#pragma once

#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
    class ENGINE_EXPORT SpotLightComponentProxy : public LightComponentProxy
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

    class ENGINE_EXPORT SpotLightComponent : public LightComponent
    {
        declare_class(SpotLightComponent, LightComponent);

    private:
        float m_angle;

        void on_angle_changed();

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
