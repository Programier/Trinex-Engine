#pragma once

#include <Engine/ActorComponents/local_light_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT PointLightComponentProxy : public LocalLightComponentProxy
    {
    private:
        float m_fall_off_exponent;

    public:
        float fall_off_exponent() const;
        PointLightComponentProxy& fall_off_exponent(float value);
        friend class PointLightComponent;
    };

    class ENGINE_EXPORT PointLightComponent : public LocalLightComponent
    {
        declare_class(PointLightComponent, LocalLightComponent);

        float m_fall_off_exponent;

        PointLightComponent& submit_point_light_data();

    public:
        PointLightComponent();
        float fall_off_exponent() const;
        PointLightComponent& fall_off_exponent(float value);

        Type light_type() const override;

        PointLightComponent& start_play() override;
        PointLightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        PointLightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
        ActorComponentProxy* create_proxy() override;
        PointLightComponentProxy* proxy() const;
    };

}// namespace Engine
