#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
    class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
    {
        declare_class(PrimitiveComponent, SceneComponent);

    protected:
        bool m_is_visible;
        AABB_3Df m_bounding_box;
        class SceneLayer* m_layer = nullptr;

    public:
        bool is_visible() const;
        const AABB_3Df& bounding_box() const;

        PrimitiveComponent& spawned() override;
        PrimitiveComponent& destroyed() override;

        virtual PrimitiveComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer);
        virtual PrimitiveComponent& render(class SceneRenderer*, class RenderViewport*, class SceneLayer*);

        friend class SceneLayer;
    };
}// namespace Engine
