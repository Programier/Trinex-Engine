#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>


namespace Engine
{
    class ENGINE_EXPORT LightComponentProxy : public SceneComponentProxy
    {
    protected:
        AABB_3Df m_bounds;

    public:
        LightComponentProxy& bounding_box(const AABB_3Df& bounds);
        const AABB_3Df& bounding_box() const;
        friend class LightComponent;
    };

    class ENGINE_EXPORT LightComponent : public SceneComponent
    {
        declare_class(LightComponent, SceneComponent);

    private:
        AABB_3Df m_bounds;
        class SceneLayer* m_layer = nullptr;

    public:
        enum Type
        {
            Unknown     = -1,
            Point       = 0,
            Spot        = 1,
            Directional = 2,
            Num         = 3
        };

    protected:
        void submit_bounds_to_render_thread();

    public:
        bool is_enabled;
        Color3 light_color;
        float intensivity;

        LightComponent();
        virtual Type light_type() const = 0;
        virtual LightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer);
        virtual LightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*);
        ActorComponentProxy* create_proxy() override;
        LightComponentProxy* proxy() const;

        LightComponent& on_transform_changed() override;
        LightComponent& start_play() override;
        LightComponent& stop_play() override;
        const AABB_3Df& bounding_box() const;
        LightComponent& update_bounding_box();
        SceneLayer* scene_layer() const;
        ~LightComponent();

        friend class SceneLayer;
    };
}// namespace Engine
