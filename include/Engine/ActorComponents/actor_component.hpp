#pragma once
#include <Core/flags.hpp>
#include <Core/object.hpp>


namespace Engine
{
    class Actor;

    class ENGINE_EXPORT ActorComponentProxy
    {
    public:
        ActorComponentProxy();
        virtual ~ActorComponentProxy();
    };

    class ENGINE_EXPORT ActorComponent : public Object
    {
        declare_class(ActorComponent, Object);

    private:
        ActorComponentProxy* m_proxy;


        void destroy_proxy();

    public:
        enum Flag
        {
            DisableRaycast = BIT(0),
        };

        Flags<Flag, Atomic<BitMask>> component_flags;

        ActorComponent();
        ~ActorComponent();

        virtual ActorComponent& start_play();
        virtual ActorComponent& stop_play();
        virtual ActorComponent& update(float dt);
        virtual ActorComponent& spawned();
        virtual ActorComponent& destroyed();
        virtual ActorComponentProxy* create_proxy();
        ActorComponentProxy* proxy() const;

        template<typename ProxyType>
        ProxyType* typed_proxy() const
        {
            return reinterpret_cast<ProxyType*>(proxy());
        }

        class Actor* actor() const;
        class World* world() const;
        class Scene* scene() const;
        class ActorComponent& actor(Actor* actor);
    };
}// namespace Engine
