#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>
#include <Event/event.hpp>


namespace Engine
{
    class ENGINE_EXPORT EventSystem : public Singletone<EventSystem, Engine::System>
    {
        declare_class(EventSystem, System);

    public:
        using ListenerSignature = void(const Event&);
        using Listener          = Function<ListenerSignature>;
        using ListenerMap       = TreeMap<byte, CallBacks<ListenerSignature>>;


    private:
        ListenerMap _M_listeners;

        EventSystem();

    public:
        const ListenerMap& listeners() const;
        Identifier add_listener(EventType event_type, const Listener& listener);
        EventSystem& remove_listener(EventType event_type, Identifier id);
        EventSystem& create() override;
        EventSystem& update(float dt) override;
        const EventSystem& push_event(const Event& event) const;

        friend class Object;
    };
}// namespace Engine
