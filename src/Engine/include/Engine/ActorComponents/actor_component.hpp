#pragma once
#include <Core/object.hpp>


namespace Engine
{
    class Actor;

    class ENGINE_EXPORT ActorComponent : public Object
    {
        declare_class(ActorComponent, Object);

    public:
        virtual ActorComponent& begin_play();
        virtual ActorComponent& end_play();
        virtual ActorComponent& update(float InDeltaTime);
        virtual ActorComponent& spawned();
        virtual ActorComponent& destroyed();
        class Actor* owner() const;
    };
}// namespace Engine
