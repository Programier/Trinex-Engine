#pragma once

#include <Engine/ActorComponents/light_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT PointLightComponent : public LightComponent
    {
        declare_class(PointLightComponent, LightComponent);

    public:
        PointLightComponent();
        Type light_type() const override;

        PointLightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        PointLightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
    };

}// namespace Engine
