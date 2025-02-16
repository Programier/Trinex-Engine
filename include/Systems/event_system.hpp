#pragma once
#include <Core/callback.hpp>
#include <Core/etl/list.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/event.hpp>
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
		struct ListenerNode {
			Listener listener;
			EventType type;
			ListenerNode* next = nullptr;
			ListenerNode* prev = nullptr;

			inline size_t index() const
			{
				return static_cast<size_t>(type);
			}

			inline Identifier id() const
			{
				return reinterpret_cast<Identifier>(this);
			}
		};

		ListenerNode* m_listeners[static_cast<size_t>(EventType::COUNT)];

		EventSystem& (EventSystem::*m_process_events)() = nullptr;

		EventSystem& wait_events();
		EventSystem& pool_events();
		EventSystem();

		EventSystem& execute_listeners(ListenerNode* node, const Event& event);

	protected:
		EventSystem& create() override;

	public:
		Identifier add_listener(EventType event_type, const Listener& listener);
		EventSystem& remove_listener(Identifier);
		EventSystem& update(float dt) override;
		EventSystem& push_event(const Event& event);
		EventSystem& shutdown() override;
		static Name event_name(EventType type);

		EventSystem& process_event_method(ProcessEventMethod method);

		friend class Singletone<EventSystem, Engine::System>;
	};
}// namespace Engine
