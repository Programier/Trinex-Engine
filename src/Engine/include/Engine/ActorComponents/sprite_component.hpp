#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT SpriteComponent : public PrimitiveComponent
    {
        declare_class(SpriteComponent, PrimitiveComponent);

        class Texture2D* m_texture = nullptr;

    public:
        Texture2D* texture() const;
        SpriteComponent& texture(Texture2D* texture);
        SpriteComponent& update_bounding_box() override;

        SpriteComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        SpriteComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*) override;
    };
}// namespace Engine
