#include <Core/class.hpp>
#include <Engine/ActorComponents/actor_component.hpp>
#include <Engine/Actors/actor.hpp>

namespace Engine
{
    implement_engine_class_default_init(ActorComponent);

    ActorComponentProxy::ActorComponentProxy()
    {}

    ActorComponentProxy::~ActorComponentProxy()
    {}

    ActorComponent::ActorComponent() : m_proxy(nullptr)
    {}

    ActorComponent::~ActorComponent()
    {
        destroy_proxy();
    }

    void ActorComponent::destroy_proxy()
    {
        if (m_proxy)
        {
            delete m_proxy;
            m_proxy = nullptr;
        }
    }

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
        m_proxy = create_proxy();
        return *this;
    }

    ActorComponent& ActorComponent::destroyed()
    {
        destroy_proxy();
        return *this;
    }

    ActorComponentProxy* ActorComponent::create_proxy()
    {
        return nullptr;
    }

    ActorComponentProxy* ActorComponent::proxy() const
    {
        return m_proxy;
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
