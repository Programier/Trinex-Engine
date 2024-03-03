#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>


namespace Engine
{
    class ENGINE_EXPORT LightComponent : public SceneComponent
    {
        declare_class(LightComponent, SceneComponent);

    private:
        AABB_3Df m_aabb;
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


    public:
        bool is_enabled;
        Color3 light_color;
        float intensivity;

        LightComponent();
        virtual Type light_type() const = 0;
        virtual LightComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer);
        virtual LightComponent& render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*);
        LightComponent& on_transform_changed() override;
        LightComponent& spawned() override;
        LightComponent& destroyed() override;
        const AABB_3Df& bounding_box() const;
        SceneLayer* scene_layer() const;
        ~LightComponent();

        friend class SceneLayer;
    };
}// namespace Engine
