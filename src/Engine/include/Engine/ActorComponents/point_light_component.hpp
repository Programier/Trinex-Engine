#pragma once

#include <Engine/ActorComponents/local_light_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT PointLightComponentProxy : public LocalLightComponentProxy
    {
    public:
        friend class PointLightComponent;
    };

    class ENGINE_EXPORT PointLightComponent : public LocalLightComponent
    {
        declare_class(PointLightComponent, LocalLightComponent);


        PointLightComponent& submit_point_light_data();

    public:
        PointLightComponent();
        Type light_type() const override;

        PointLightComponent& start_play() override;
        PointLightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        PointLightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
        ActorComponentProxy* create_proxy() override;
        PointLightComponentProxy* proxy() const;
    };

}// namespace Engine
