#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>

namespace Engine
{
    implement_engine_class_default_init(PrimitiveComponent);
    static const AABB_3Df default_bounds({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});

    PrimitiveComponentProxy& PrimitiveComponentProxy::bounding_box(const AABB_3Df& bounds)
    {
        m_bounds = bounds;
        return *this;
    }


    const AABB_3Df& PrimitiveComponentProxy::bounding_box() const
    {
        return m_bounds;
    }


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

    ActorComponentProxy* PrimitiveComponent::create_proxy()
    {
        return new PrimitiveComponentProxy();
    }

    PrimitiveComponent& PrimitiveComponent::on_transform_changed()
    {
        Super::on_transform_changed();

        if (Scene* world_scene = scene())
        {
            world_scene->update_primitive_transform(this);
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
            static const ByteColor red = {255, 0, 0, 255};
            proxy()->bounding_box().write_to_batcher(layer->lines, red);
        }
        return *this;
    }

    PrimitiveComponentProxy* PrimitiveComponent::proxy() const
    {
        return typed_proxy<PrimitiveComponentProxy>();
    }

    PrimitiveComponent& PrimitiveComponent::update_bounding_box()
    {
        m_bounding_box = default_bounds.apply_transform(world_transform().matrix());
        return *this;
    }


    PrimitiveComponent::~PrimitiveComponent()
    {}
}// namespace Engine
