#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>

namespace Engine
{
    implement_engine_class_default_init(PrimitiveComponent);
    static const AABB_3Df default_bounds({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});

    PrimitiveComponent::PrimitiveComponent() : m_bounding_box(default_bounds)
    {}

    bool PrimitiveComponent::is_visible() const
    {
        return m_is_visible;
    }

    const AABB_3Df& PrimitiveComponent::bounding_box() const
    {
        return m_bounding_box;
    }

    PrimitiveComponent& PrimitiveComponent::spawned()
    {
        Super::spawned();
        if (Actor* owner_actor = actor())
        {
            if (World* world = owner_actor->world())
            {
                if (Scene* scene = world->scene())
                {
                    scene->add_primitive(this);
                }
            }
        }
        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::destroyed()
    {
        Super::destroyed();

        if (Actor* owner_actor = actor())
        {
            if (World* world = owner_actor->world())
            {
                if (Scene* scene = world->scene())
                {
                    scene->remove_primitive(this);
                }
            }
        }
        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::on_transform_changed()
    {
        Super::on_transform_changed();

        if (Actor* owner_actor = actor())
        {
            if (World* world = owner_actor->world())
            {
                if (Scene* scene = world->scene())
                {
                    scene->remove_primitive(this);
                    update_bounding_box();
                    scene->add_primitive(this);
                }
            }
        }

        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::render(class SceneRenderer*, class RenderTargetBase*, class SceneLayer* layer)
    {
        if (engine_instance->is_editor())
        {
            const auto& min = m_bounding_box.min();
            const auto& max = m_bounding_box.max();
            static const ByteColor red = {255, 0, 0, 255};

            layer->lines.add_line({min.x, min.y, min.z}, {max.x, min.y, min.z}, red, red);
            layer->lines.add_line({min.x, max.y, min.z}, {max.x, max.y, min.z}, red, red);
            layer->lines.add_line({min.x, min.y, max.z}, {max.x, min.y, max.z}, red, red);
            layer->lines.add_line({min.x, max.y, max.z}, {max.x, max.y, max.z}, red, red);

            layer->lines.add_line({min.x, min.y, min.z}, {min.x, max.y, min.z}, red, red);
            layer->lines.add_line({max.x, min.y, min.z}, {max.x, max.y, min.z}, red, red);
            layer->lines.add_line({min.x, min.y, max.z}, {min.x, max.y, max.z}, red, red);
            layer->lines.add_line({max.x, min.y, max.z}, {max.x, max.y, max.z}, red, red);

            layer->lines.add_line({min.x, min.y, min.z}, {min.x, min.y, max.z}, red, red);
            layer->lines.add_line({max.x, min.y, min.z}, {max.x, min.y, max.z}, red, red);
            layer->lines.add_line({min.x, max.y, min.z}, {min.x, max.y, max.z}, red, red);
            layer->lines.add_line({max.x, max.y, min.z}, {max.x, max.y, max.z}, red, red);
        }
        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::update_bounding_box()
    {
        static auto min = Vector3D(transform.local_to_world * Vector4D(default_bounds.min(), 1.f));
        auto max        = Vector3D(transform.local_to_world * Vector4D(default_bounds.max(), 1.f));
        m_bounding_box  = AABB_3Df(min, max);
        return *this;
    }
}// namespace Engine
