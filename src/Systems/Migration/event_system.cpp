#include <Platform/platform.hpp>
#include <Systems/Migration/event_system.hpp>
#include <Systems/Migration/input_system.hpp>


namespace Trinex::Migration
{
	EventSystem* EventSystem::s_instance = nullptr;

	namespace
	{
		static RawInputEventBatch single_event_batch(const RawInputEvent& event)
		{
			RawInputEventBatch batch;
			batch.first_sequence = event.header.sequence;
			batch.last_sequence  = event.header.sequence;
			batch.events.push_back(event);
			return batch;
		}

		static void attach_raw_payload(RoutedEvent& routed, const RawInputEvent& raw_event)
		{
			switch (raw_event.type)
			{
				case RawInputEventType::Window:
					routed.payload      = const_cast<WindowEvent*>(&raw_event.window);
					routed.payload_size = sizeof(raw_event.window);
					break;

				case RawInputEventType::Key:
					routed.payload      = const_cast<KeyEvent*>(&raw_event.key);
					routed.payload_size = sizeof(raw_event.key);
					break;

				case RawInputEventType::TextInput:
					routed.payload      = const_cast<TextInputEvent*>(&raw_event.text_input);
					routed.payload_size = sizeof(raw_event.text_input);
					break;

				case RawInputEventType::Pointer:
					routed.payload      = const_cast<PointerEvent*>(&raw_event.pointer);
					routed.payload_size = sizeof(raw_event.pointer);
					break;

				case RawInputEventType::Gamepad:
					routed.payload      = const_cast<GamepadEvent*>(&raw_event.gamepad);
					routed.payload_size = sizeof(raw_event.gamepad);
					break;

				case RawInputEventType::DeviceChange:
					routed.payload      = const_cast<DeviceChangeEvent*>(&raw_event.device_change);
					routed.payload_size = sizeof(raw_event.device_change);
					break;

				case RawInputEventType::None:
				default:
					routed.payload      = nullptr;
					routed.payload_size = 0;
					break;
			}
		}

		static EventDispatchResult dispatch_raw_event(EventDispatcher& dispatcher, const RawInputEvent& raw_event)
		{
			if (raw_event.header.type_id == 0)
			{
				return {};
			}

			RoutedEvent routed(raw_event.header);
			attach_raw_payload(routed, raw_event);
			EventDispatchResult result = dispatcher.dispatch(routed);
			routed.result              = result;
			return result;
		}
	}// namespace

	EventTarget* EventTarget::event_parent() const
	{
		return nullptr;
	}

	EventTarget::ListenerList& EventTarget::listeners_for_phase(EventPhase phase)
	{
		switch (phase)
		{
			case EventPhase::Preview: return m_preview_listeners;
			case EventPhase::Capture: return m_capture_listeners;
			case EventPhase::Target: return m_target_listeners;
			case EventPhase::Bubble: return m_bubble_listeners;
			case EventPhase::Unhandled: return m_unhandled_listeners;
			case EventPhase::Post:
			case EventPhase::None:
			default: return m_post_listeners;
		}
	}

	const EventTarget::ListenerList& EventTarget::listeners_for_phase(EventPhase phase) const
	{
		switch (phase)
		{
			case EventPhase::Preview: return m_preview_listeners;
			case EventPhase::Capture: return m_capture_listeners;
			case EventPhase::Target: return m_target_listeners;
			case EventPhase::Bubble: return m_bubble_listeners;
			case EventPhase::Unhandled: return m_unhandled_listeners;
			case EventPhase::Post:
			case EventPhase::None:
			default: return m_post_listeners;
		}
	}

	EventTarget& EventTarget::add_listener(EventPhase phase, EventListener* listener)
	{
		if (listener)
		{
			listeners_for_phase(phase).push_back(listener);
		}

		return *this;
	}

	EventTarget& EventTarget::remove_listener(EventPhase phase, EventListener* listener)
	{
		ListenerList& items = listeners_for_phase(phase);

		for (auto it = items.begin(); it != items.end(); ++it)
		{
			if (*it == listener)
			{
				items.erase(it);
				break;
			}
		}

		return *this;
	}

	const EventTarget::ListenerList& EventTarget::listeners(EventPhase phase) const
	{
		return listeners_for_phase(phase);
	}

	EventDispatchResult EventTarget::on_preview_event(RoutedEvent& event)
	{
		EventDispatchResult result;

		for (EventListener* listener : m_preview_listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			result.handled |= current.handled;
			result.consumed |= current.consumed;
			result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				result.continue_propagation = false;
				break;
			}
		}

		return result;
	}

	EventDispatchResult EventTarget::on_capture_event(RoutedEvent& event)
	{
		EventDispatchResult result;

		for (EventListener* listener : m_capture_listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			result.handled |= current.handled;
			result.consumed |= current.consumed;
			result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				result.continue_propagation = false;
				break;
			}
		}

		return result;
	}

	EventDispatchResult EventTarget::on_target_event(RoutedEvent& event)
	{
		EventDispatchResult result;

		for (EventListener* listener : m_target_listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			result.handled |= current.handled;
			result.consumed |= current.consumed;
			result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				result.continue_propagation = false;
				break;
			}
		}

		return result;
	}

	EventDispatchResult EventTarget::on_bubble_event(RoutedEvent& event)
	{
		EventDispatchResult result;

		for (EventListener* listener : m_bubble_listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			result.handled |= current.handled;
			result.consumed |= current.consumed;
			result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				result.continue_propagation = false;
				break;
			}
		}

		return result;
	}

	EventDispatchResult EventTarget::on_unhandled_event(RoutedEvent& event)
	{
		EventDispatchResult result;

		for (EventListener* listener : m_unhandled_listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			result.handled |= current.handled;
			result.consumed |= current.consumed;
			result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				result.continue_propagation = false;
				break;
			}
		}

		return result;
	}

	EventDispatchResult EventTarget::on_post_event(RoutedEvent& event)
	{
		EventDispatchResult result;

		for (EventListener* listener : m_post_listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			result.handled |= current.handled;
			result.consumed |= current.consumed;
			result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				result.continue_propagation = false;
				break;
			}
		}

		return result;
	}

	EventQueue& EventQueue::push(const Event& event)
	{
		m_events.push_back(event);
		return *this;
	}

	EventQueue& EventQueue::clear()
	{
		m_events.clear();
		return *this;
	}

	usize EventQueue::size() const
	{
		return m_events.size();
	}

	bool EventQueue::empty() const
	{
		return m_events.empty();
	}

	const Vector<Event>& EventQueue::events() const
	{
		return m_events;
	}

	DeferredEventQueue& DeferredEventQueue::enqueue(const Event& event)
	{
		m_messages.push_back(event);
		return *this;
	}

	DeferredEventQueue& DeferredEventQueue::clear()
	{
		m_messages.clear();
		return *this;
	}

	DeferredEventQueue& DeferredEventQueue::drain_to(Vector<Event>& output)
	{
		for (const Event& event : m_messages)
		{
			output.push_back(event);
		}

		m_messages.clear();
		return *this;
	}

	usize DeferredEventQueue::size() const
	{
		return m_messages.size();
	}

	bool DeferredEventQueue::empty() const
	{
		return m_messages.empty();
	}

	const Vector<Event>& DeferredEventQueue::messages() const
	{
		return m_messages;
	}

	EventDispatchResult EventDispatcher::merge_result(EventDispatchResult& destination, const EventDispatchResult& source)
	{
		destination.handled |= source.handled;
		destination.consumed |= source.consumed;
		destination.emit_gameplay_actions &= source.emit_gameplay_actions;

		if (!source.continue_propagation)
		{
			destination.continue_propagation = false;
		}

		if (source.terminal_phase != EventPhase::None)
		{
			destination.terminal_phase = source.terminal_phase;
		}

		return destination;
	}

	void EventDispatcher::finalize_event_result(RoutedEvent& event)
	{
		if (event.result.handled)
		{
			event.header.flags |= EventFlags::Handled;
		}

		if (event.result.consumed)
		{
			event.header.flags |= EventFlags::Consumed;
			event.result.continue_propagation = false;
		}

		if (event.result.terminal_phase == EventPhase::None)
		{
			event.result.terminal_phase = event.phase;
		}
	}

	bool EventDispatcher::should_dispatch_phase(const RoutedEvent& event, EventPhase phase)
	{
		switch (phase)
		{
			case EventPhase::Preview: return has_event_flags(event.header.flags, EventFlags::AllowPreview);
			case EventPhase::Capture: return has_event_flags(event.header.flags, EventFlags::AllowCapture);
			case EventPhase::Bubble: return has_event_flags(event.header.flags, EventFlags::AllowBubble);
			case EventPhase::Target:
			case EventPhase::Unhandled:
			case EventPhase::Post: return true;
			case EventPhase::None:
			default: return false;
		}
	}

	Vector<EventTarget*> EventDispatcher::build_route(EventTarget* target, EventTarget* routing_root) const
	{
		Vector<EventTarget*> route;
		EventTarget* current = target;

		while (current)
		{
			route.push_back(current);

			if (current == routing_root)
			{
				break;
			}

			current = current->event_parent();
		}

		return route;
	}

	EventDispatchResult EventDispatcher::dispatch_listeners(const ListenerList& listeners, RoutedEvent& event) const
	{
		EventDispatchResult result;

		for (EventListener* listener : listeners)
		{
			if (listener == nullptr)
			{
				continue;
			}

			EventDispatchResult current = listener->on_event(event);
			merge_result(result, current);

			if (!current.continue_propagation)
			{
				break;
			}
		}

		return result;
	}

	EventDispatchResult EventDispatcher::notify_listeners(RoutedEvent& event)
	{
		if (auto found = m_listeners.find(event.header.type_id); found != m_listeners.end())
		{
			return dispatch_listeners(found->second, event);
		}

		return {};
	}

	EventDispatchResult EventDispatcher::dispatch_single_phase(RoutedEvent& event, EventPhase phase, EventTarget* target)
	{
		event.phase          = phase;
		event.current_target = target;

		if (target == nullptr)
		{
			return {};
		}

		switch (phase)
		{
			case EventPhase::Preview: return target->on_preview_event(event);
			case EventPhase::Capture: return target->on_capture_event(event);
			case EventPhase::Target: return target->on_target_event(event);
			case EventPhase::Bubble: return target->on_bubble_event(event);
			case EventPhase::Unhandled: return target->on_unhandled_event(event);
			case EventPhase::Post: return target->on_post_event(event);
			case EventPhase::None:
			default: return {};
		}
	}

	EventDispatcher& EventDispatcher::add_listener(EventTypeId type_id, EventListener* listener)
	{
		m_listeners[type_id].push_back(listener);
		return *this;
	}

	EventDispatcher& EventDispatcher::remove_listener(EventTypeId type_id, EventListener* listener)
	{
		if (auto found = m_listeners.find(type_id); found != m_listeners.end())
		{
			auto& listeners = found->second;

			for (auto it = listeners.begin(); it != listeners.end(); ++it)
			{
				if (*it == listener)
				{
					listeners.erase(it);
					break;
				}
			}
		}

		return *this;
	}

	EventDispatchResult EventDispatcher::dispatch(RoutedEvent& event)
	{
		return dispatch_to_target(event, event.target);
	}

	EventDispatchResult EventDispatcher::dispatch_to_target(RoutedEvent& event, EventTarget* target)
	{
		event.target = target;
		event.phase  = EventPhase::None;
		event.route  = build_route(target, event.routing_root);
		event.result = notify_listeners(event);
		finalize_event_result(event);

		if (!event.result.continue_propagation)
		{
			return event.result;
		}

		if (should_dispatch_phase(event, EventPhase::Preview))
		{
			for (auto it = event.route.rbegin(); it != event.route.rend(); ++it)
			{
				EventDispatchResult current = dispatch_single_phase(event, EventPhase::Preview, *it);
				merge_result(event.result, current);
				finalize_event_result(event);

				if (!current.continue_propagation)
				{
					event.result.terminal_phase = EventPhase::Preview;
					return event.result;
				}
			}
		}

		if (should_dispatch_phase(event, EventPhase::Capture))
		{
			for (auto it = event.route.rbegin(); it != event.route.rend(); ++it)
			{
				EventDispatchResult current = dispatch_single_phase(event, EventPhase::Capture, *it);
				merge_result(event.result, current);
				finalize_event_result(event);

				if (!current.continue_propagation)
				{
					event.result.terminal_phase = EventPhase::Capture;
					return event.result;
				}
			}
		}

		EventDispatchResult target_result = dispatch_single_phase(event, EventPhase::Target, target);
		merge_result(event.result, target_result);
		finalize_event_result(event);

		if (!target_result.continue_propagation)
		{
			event.result.terminal_phase = EventPhase::Target;
			return event.result;
		}

		if (should_dispatch_phase(event, EventPhase::Bubble))
		{
			for (EventTarget* route_target : event.route)
			{
				EventDispatchResult current = dispatch_single_phase(event, EventPhase::Bubble, route_target);
				merge_result(event.result, current);
				finalize_event_result(event);

				if (!current.continue_propagation)
				{
					event.result.terminal_phase = EventPhase::Bubble;
					return event.result;
				}
			}
		}

		if (!event.result.handled)
		{
			EventDispatchResult current = dispatch_single_phase(event, EventPhase::Unhandled, target);
			merge_result(event.result, current);
			finalize_event_result(event);

			if (!current.continue_propagation)
			{
				event.result.terminal_phase = EventPhase::Unhandled;
				return event.result;
			}
		}

		for (EventTarget* route_target : event.route)
		{
			EventDispatchResult post = dispatch_single_phase(event, EventPhase::Post, route_target);
			merge_result(event.result, post);
			finalize_event_result(event);

			if (!post.continue_propagation)
			{
				event.result.terminal_phase = EventPhase::Post;
				return event.result;
			}
		}

		return event.result;
	}

	EventDispatcher& EventSystem::dispatcher()
	{
		return m_dispatcher;
	}

	DeferredEventQueue& EventSystem::deferred_messages()
	{
		return m_deferred_messages;
	}

	EventQueue& EventSystem::event_queue()
	{
		return m_event_queue;
	}

	EventHeader EventSystem::make_header(EventTypeId type_id, EventFlags flags)
	{
		EventHeader header;
		header.type_id  = type_id;
		header.flags    = flags;
		header.sequence = m_next_sequence++;
		return header;
	}

	EventDispatchResult EventSystem::route(RoutedEvent& event)
	{
		m_event_queue.push(event);
		EventDispatchResult result = m_dispatcher.dispatch(event);
		event.result               = result;
		return result;
	}

	EventSystem& EventSystem::submit_raw_event(const RawInputEvent& event)
	{
		m_event_queue.push(Event(event.header));

		auto input_system = InputSystem::instance();

		RawInputEventBatch batch = single_event_batch(event);
		input_system->submit_raw_event(event);
		input_system->update_device_state(batch);

		dispatch_raw_event(m_dispatcher, event);

		return *this;
	}

	EventSystem& EventSystem::submit_raw_event_batch(const RawInputEventBatch& batch)
	{
		for (const RawInputEvent& event : batch.events)
		{
			m_event_queue.push(Event(event.header));
		}

		auto input_system = InputSystem::instance();

		input_system->submit_raw_event_batch(batch);
		input_system->update_device_state(batch);

		for (const RawInputEvent& event : batch.events)
		{
			dispatch_raw_event(m_dispatcher, event);
		}

		return *this;
	}

	EventSystem& EventSystem::queue_deferred(const Event& event)
	{
		m_deferred_messages.enqueue(event);
		return *this;
	}

	EventSystem& EventSystem::begin_frame()
	{
		// TODO(Migration): Move queued deferred messages into a deterministic gameplay tick buffer.
		m_event_queue.clear();
		return *this;
	}

	EventSystem& EventSystem::update(float dt)
	{
		Platform::EventSystem::pool_events();
		return *this;
	}

	EventSystem& EventSystem::end_frame()
	{
		return *this;
	}
}// namespace Trinex::Migration
