#pragma once

#include <Engine/ActorComponents/light_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT SpotLightComponent : public LightComponent
    {
        declare_class(SpotLightComponent, LightComponent);

    public:
        float radius;
        float height;

        SpotLightComponent();
        Type light_type() const override;

        SpotLightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        SpotLightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
    };

}// namespace Engine
