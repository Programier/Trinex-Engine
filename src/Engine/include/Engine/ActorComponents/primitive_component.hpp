#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
    class PrimitiveComponent;

    class ENGINE_EXPORT PrimitiveComponentProxy : public SceneComponentProxy
    {
    protected:
        AABB_3Df m_bounds;

    public:
        PrimitiveComponentProxy& bounding_box(const AABB_3Df& bounds);
        const AABB_3Df& bounding_box() const;
        friend class PrimitiveComponent;
    };

    class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
    {
        declare_class(PrimitiveComponent, SceneComponent);

    protected:
        bool m_is_visible;
        AABB_3Df m_bounding_box;
        class SceneLayer* m_layer = nullptr;

        void submit_bounds_to_render_thread();

    public:
        PrimitiveComponent();
        bool is_visible() const;
        const AABB_3Df& bounding_box() const;

        PrimitiveComponent& start_play() override;
        PrimitiveComponent& stop_play() override;
        PrimitiveComponent& on_transform_changed() override;
        ActorComponentProxy* create_proxy() override;

        virtual PrimitiveComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer);
        virtual PrimitiveComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*);
        virtual PrimitiveComponent& update_bounding_box();

        PrimitiveComponentProxy* proxy() const;
        ~PrimitiveComponent();
        friend class SceneLayer;
    };
}// namespace Engine
