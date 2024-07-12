#pragma once
#include <Core/etl/singletone.hpp>
#include <Event/listener_id.hpp>
#include <Systems/system.hpp>


namespace Engine
{
    class ENGINE_EXPORT EventSystem : public Singletone<EventSystem, Engine::System>
    {
        declare_class(EventSystem, System);

    public:
        using ListenerSignature = void(const Event&);
        using Listener          = Function<ListenerSignature>;
        using ListenerMap       = TreeMap<EnumerateType, CallBacks<ListenerSignature>>;

        enum ProcessEventMethod
        {
            PoolEvents,
            WaitingEvents,
        };


    private:
        ListenerMap m_listeners;
        EventSystem& (EventSystem::*m_process_events)() = nullptr;
        List<Identifier> m_windows_to_destroy;
        List<EventSystemListenerID> m_listeners_to_remove;


        EventSystem& wait_events();
        EventSystem& pool_events();
        EventSystem();

        void on_window_close(const Event& event, bool is_quit);

    public:
        const ListenerMap& listeners() const;
        EventSystemListenerID add_listener(EventType event_type, const Listener& listener);
        EventSystem& remove_listener(const EventSystemListenerID&);
        EventSystem& create() override;
        EventSystem& update(float dt) override;
        const EventSystem& push_event(const Event& event) const;
        EventSystem& shutdown() override;
        static Name event_name(EventType type);

        EventSystem& process_event_method(ProcessEventMethod method);

        friend class Singletone<EventSystem, Engine::System>;
    };
}// namespace Engine
