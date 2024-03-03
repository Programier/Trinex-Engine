#include <Core/class.hpp>
#include <Engine/ActorComponents/actor_component.hpp>
#include <Engine/Actors/actor.hpp>

namespace Engine
{
    implement_engine_class_default_init(ActorComponent);

    ActorComponent& ActorComponent::begin_play()
    {
        return *this;
    }

    ActorComponent& ActorComponent::end_play()
    {
        return *this;
    }

    ActorComponent& ActorComponent::update(float dt)
    {
        return *this;
    }

    ActorComponent& ActorComponent::spawned()
    {
        return *this;
    }

    ActorComponent& ActorComponent::destroyed()
    {
        return *this;
    }

    class Actor* ActorComponent::actor() const
    {
        return Super::owner()->instance_cast<Actor>();
    }

    class World* ActorComponent::world() const
    {
        Actor* owner_actor = actor();
        return owner_actor ? owner_actor->world() : nullptr;
    }

    class Scene* ActorComponent::scene() const
    {
        Actor* owner_actor = actor();
        return owner_actor ? owner_actor->scene() : nullptr;
    }

    class ActorComponent& ActorComponent::actor(Actor* actor)
    {
        owner(actor);
        return *this;
    }
}// namespace Engine
