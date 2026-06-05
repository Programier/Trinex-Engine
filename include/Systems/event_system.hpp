#pragma once

#include <Core/callback.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/etl/vector.hpp>
#include <Core/event.hpp>

namespace Trinex
{
	class ENGINE_EXPORT EventSystem : public Singletone<EventSystem, EmptySingletoneParent>
	{
	public:
		using Listener = CallBack<void(const Event&)>;

		enum ProcessEventMethod : u8
		{
			PoolEvents,
			WaitEvents,
		};

		static EventSystem* s_instance;

	private:
		friend class Singletone<EventSystem, EmptySingletoneParent>;

		struct ListenerEntry {
			Identifier id = 0;
			EventType type = EventType::Undefined;
			Listener listener;
		};

		Map<Identifier, EventType> m_listener_types;
		Vector<ListenerEntry> m_listeners[static_cast<usize>(EventType::COUNT)];
		Identifier m_next_listener_id = 1;

		EventSystem();
		void dispatch(const Event& event);

	public:
		EventSystem& begin_frame();
		EventSystem& end_frame();
		EventSystem& process_event_method(ProcessEventMethod method);
		Identifier add_listener(EventType type, const Listener& listener);
		EventSystem& remove_listener(Identifier id);
		EventSystem& push_event(const Event& event);
	};
}// namespace Trinex
