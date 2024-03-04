#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_engine_class(LightComponent, 0);
    implement_initialize_class(LightComponent)
    {
        Class* self = static_class_instance();
        self->add_properties(new BoolProperty("Is Enabled", "Is light enabled", &This::is_enabled),
                             new Color3Property("Color", "Color of this light", &This::light_color),
                             new FloatProperty("Intensivity", "Intensivity of this light", &This::intensivity));
    }

    LightComponent::LightComponent() : is_enabled(true), light_color({1.0, 1.0, 1.0}), intensivity(22400.f)
    {}

    LightComponent& LightComponent::on_transform_changed()
    {
        Super::on_transform_changed();

        if (Scene* world_scene = scene())
        {
            world_scene->remove_light(this);
            static const AABB_3Df box({-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5});
            m_aabb = box.apply_transform(world_transform().matrix());
            world_scene->add_light(this);
        }

        return *this;
    }

    LightComponent& LightComponent::spawned()
    {
        Scene* world_scene = scene();
        if (world_scene)
        {
            world_scene->add_light(this);
        }
        return *this;
    }

    LightComponent& LightComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        return *this;
    }

    LightComponent& LightComponent::render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer*)
    {
        return *this;
    }

    LightComponent& LightComponent::destroyed()
    {
        Scene* world_scene = scene();

        if (world_scene)
        {
            world_scene->remove_light(this);
        }

        return *this;
    }

    const AABB_3Df& LightComponent::bounding_box() const
    {
        return m_aabb;
    }

    SceneLayer* LightComponent::scene_layer() const
    {
        return m_layer;
    }

    LightComponent::~LightComponent()
    {}
}// namespace Engine
