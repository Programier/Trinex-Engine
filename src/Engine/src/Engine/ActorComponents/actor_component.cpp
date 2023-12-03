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

    class Actor* ActorComponent::owner() const
    {
        return Super::owner()->instance_cast<Actor>();
    }
}// namespace Engine
