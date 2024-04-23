#pragma once
#include <Core/flags.hpp>
#include <Core/object.hpp>


namespace Engine
{
    class Actor;

    class ENGINE_EXPORT ActorComponent : public Object
    {
        declare_class(ActorComponent, Object);

    public:
        enum Flag
        {
            DisableRaycast = BIT(0),
        };

        Flags<Flag> component_flags;

        virtual ActorComponent& begin_play();
        virtual ActorComponent& end_play();
        virtual ActorComponent& update(float dt);
        virtual ActorComponent& spawned();
        virtual ActorComponent& destroyed();

        class Actor* actor() const;
        class World* world() const;
        class Scene* scene() const;
        class ActorComponent& actor(Actor* actor);
    };
}// namespace Engine
