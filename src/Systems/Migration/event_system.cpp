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
	}// namespace

	EventTarget* EventTarget::event_parent() const
	{
		return nullptr;
	}

	EventDispatchResult EventTarget::on_preview_event(RoutedEvent&)
	{
		return {};
	}

	EventDispatchResult EventTarget::on_capture_event(RoutedEvent&)
	{
		return {};
	}

	EventDispatchResult EventTarget::on_target_event(RoutedEvent&)
	{
		return {};
	}

	EventDispatchResult EventTarget::on_bubble_event(RoutedEvent&)
	{
		return {};
	}

	EventDispatchResult EventTarget::on_unhandled_event(RoutedEvent&)
	{
		return {};
	}

	EventDispatchResult EventTarget::on_post_event(RoutedEvent&)
	{
		return {};
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

	EventDispatchResult EventDispatcher::notify_listeners(RoutedEvent& event)
	{
		EventDispatchResult combined;

		if (auto found = m_listeners.find(event.header.type_id); found != m_listeners.end())
		{
			for (EventListener* listener : found->second)
			{
				if (listener == nullptr)
				{
					continue;
				}

				EventDispatchResult current = listener->on_event(event);
				combined.handled |= current.handled;
				combined.consumed |= current.consumed;
				combined.emit_gameplay_actions &= current.emit_gameplay_actions;

				if (!current.continue_propagation)
				{
					combined.continue_propagation = false;
					break;
				}
			}
		}

		return combined;
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
		event.result = notify_listeners(event);

		if (!event.result.continue_propagation)
		{
			return event.result;
		}

		// TODO(Migration): Build a real parent path and traverse preview/capture/bubble across it.
		static constexpr EventPhase phases[] = {
		        EventPhase::Preview,
		        EventPhase::Capture,
		        EventPhase::Target,
		        EventPhase::Bubble,
		};

		for (EventPhase phase : phases)
		{
			EventDispatchResult current = dispatch_single_phase(event, phase, target);
			event.result.handled |= current.handled;
			event.result.consumed |= current.consumed;
			event.result.emit_gameplay_actions &= current.emit_gameplay_actions;

			if (!current.continue_propagation)
			{
				event.result.continue_propagation = false;
				event.result.terminal_phase       = phase;
				break;
			}
		}

		if (!event.result.handled)
		{
			EventDispatchResult current = dispatch_single_phase(event, EventPhase::Unhandled, target);
			event.result.handled |= current.handled;
			event.result.consumed |= current.consumed;
			event.result.emit_gameplay_actions &= current.emit_gameplay_actions;
		}

		EventDispatchResult post = dispatch_single_phase(event, EventPhase::Post, target);
		event.result.handled |= post.handled;
		event.result.consumed |= post.consumed;
		event.result.emit_gameplay_actions &= post.emit_gameplay_actions;

		return event.result;
	}

	EventDispatcher& EventSystem::dispatcher()
	{
		return m_dispatcher;
	}

	EventSystem& EventSystem::bind_input_system(InputSystem* input_system)
	{
		m_input_system = input_system;
		return *this;
	}

	DeferredEventQueue& EventSystem::deferred_messages()
	{
		return m_deferred_messages;
	}

	EventQueue& EventSystem::event_queue()
	{
		return m_event_queue;
	}

	InputSystem* EventSystem::input_system() const
	{
		return m_input_system;
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
		return m_dispatcher.dispatch(event);
	}

	EventSystem& EventSystem::submit_raw_event(const RawInputEvent& event)
	{
		m_event_queue.push(Event(event.header));

		if (m_input_system == nullptr)
		{
			m_input_system = InputSystem::instance();
		}

		if (m_input_system)
		{
			RawInputEventBatch batch = single_event_batch(event);
			m_input_system->submit_raw_event(event);
			m_input_system->update_device_state(batch);
		}

		return *this;
	}

	EventSystem& EventSystem::submit_raw_event_batch(const RawInputEventBatch& batch)
	{
		for (const RawInputEvent& event : batch.events)
		{
			m_event_queue.push(Event(event.header));
		}

		if (m_input_system == nullptr)
		{
			m_input_system = InputSystem::instance();
		}

		if (m_input_system)
		{
			m_input_system->submit_raw_event_batch(batch);
			m_input_system->update_device_state(batch);
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

		if (m_input_system == nullptr)
		{
			m_input_system = InputSystem::instance();
		}

		if (m_input_system)
		{
			m_input_system->begin_frame();
		}

		return *this;
	}

	EventSystem& EventSystem::end_frame()
	{
		if (m_input_system == nullptr)
		{
			m_input_system = InputSystem::instance();
		}

		if (m_input_system)
		{
			m_input_system->build_command_buffer();
			m_input_system->end_frame();
		}

		return *this;
	}
}// namespace Trinex::Migration
