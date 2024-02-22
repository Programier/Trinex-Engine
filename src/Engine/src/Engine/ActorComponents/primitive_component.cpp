#include <Core/class.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>

namespace Engine
{
    implement_engine_class_default_init(PrimitiveComponent);

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
        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        return *this;
    }

    PrimitiveComponent& PrimitiveComponent::render(class SceneRenderer*, class RenderViewport*, class SceneLayer*)
    {
        return *this;
    }
}// namespace Engine
