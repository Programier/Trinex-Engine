#pragma once

#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
    class ENGINE_EXPORT SpotLightComponentProxy : public LightComponentProxy
    {
        float m_radius;
        float m_height;
        float m_cutoff;

    public:
        float radius() const;
        float height() const;
        float cutoff() const;

        SpotLightComponentProxy& radius(float value);
        SpotLightComponentProxy& height(float value);
        SpotLightComponentProxy& cutoff(float value);
        Vector3D direction() const;

        friend class SpotLightComponent;
    };

    class ENGINE_EXPORT SpotLightComponent : public LightComponent
    {
        declare_class(SpotLightComponent, LightComponent);

    private:
        float m_radius;
        float m_height;
        float m_cutoff;

        void on_data_changed();

    public:
        SpotLightComponent();

        float radius() const;
        float height() const;
        float cutoff() const;

        SpotLightComponent& radius(float value);
        SpotLightComponent& height(float value);

        Vector3D direction() const;
        SpotLightComponentProxy* proxy() const;

        Type light_type() const override;
        SpotLightComponent& spawned() override;
        SpotLightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        SpotLightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
        ActorComponentProxy* create_proxy() override;
    };

}// namespace Engine
