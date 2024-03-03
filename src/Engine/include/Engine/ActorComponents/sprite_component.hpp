#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT SpriteComponent : public PrimitiveComponent
    {
        declare_class(SpriteComponent, PrimitiveComponent);

    public:
        class Texture2D* texture = nullptr;

        SpriteComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        SpriteComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
    };
}// namespace Engine
