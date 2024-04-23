#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
    class PrimitiveComponent;

    class ENGINE_EXPORT PrimitiveProxy
    {
    protected:
        PrimitiveComponent* m_component;
        Transform m_world_transform;
        Transform m_local_transform;
        AABB_3Df m_bounds;

    public:
        PrimitiveProxy(class PrimitiveComponent* component);
        PrimitiveProxy& world_transform(const Transform& transform);
        PrimitiveProxy& local_transform(const Transform& transform);
        PrimitiveProxy& bounding_box(const AABB_3Df& bounds);
        const Transform& world_transform() const;
        const Transform& local_transform() const;
        const AABB_3Df& bounding_box() const;

        virtual ~PrimitiveProxy();
    };

    class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
    {
        declare_class(PrimitiveComponent, SceneComponent);

    protected:
        PrimitiveProxy* m_proxy;
        bool m_is_visible;
        AABB_3Df m_bounding_box;
        class SceneLayer* m_layer = nullptr;

    public:
        PrimitiveComponent();
        bool is_visible() const;
        const AABB_3Df& bounding_box() const;

        PrimitiveComponent& spawned() override;
        PrimitiveComponent& destroyed() override;
        PrimitiveComponent& on_transform_changed() override;

        virtual PrimitiveComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer);
        virtual PrimitiveComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*);
        virtual PrimitiveComponent& update_bounding_box();

        PrimitiveProxy* proxy() const;
        virtual PrimitiveProxy* create_proxy();
        ~PrimitiveComponent();
        friend class SceneLayer;
    };
}// namespace Engine
