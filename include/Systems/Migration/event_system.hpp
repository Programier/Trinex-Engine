#pragma once

#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Core/tickable.hpp>

namespace Trinex::Migration
{
	using EventTypeId    = u64;
	using EventSequence  = u64;
	using EventTimestamp = u64;

	namespace EventTypeIds
	{
		inline constexpr EventTypeId Quit         = 1;
		inline constexpr EventTypeId Window       = 2;
		inline constexpr EventTypeId Key          = 3;
		inline constexpr EventTypeId TextInput    = 4;
		inline constexpr EventTypeId Pointer      = 5;
		inline constexpr EventTypeId Gamepad      = 6;
		inline constexpr EventTypeId DeviceChange = 7;
	}// namespace EventTypeIds

	class EventListener;
	class EventTarget;
	struct RawInputEvent;
	struct RawInputEventBatch;

	enum class EventPhase : u8
	{
		None,
		Preview,
		Capture,
		Target,
		Bubble,
		Unhandled,
		Post,
	};

	enum class EventFlags : u32
	{
		None                = 0,
		Routed              = BIT(0),
		Deferred            = BIT(1),
		Handled             = BIT(2),
		Consumed            = BIT(3),
		AllowPreview        = BIT(4),
		AllowCapture        = BIT(5),
		AllowBubble         = BIT(6),
		PointerEvent        = BIT(7),
		KeyboardEvent       = BIT(8),
		TextInputEvent      = BIT(9),
		WindowEvent         = BIT(10),
		AllowGameplayOutput = BIT(11),
	};

	inline constexpr EventFlags operator|(EventFlags lhs, EventFlags rhs)
	{
		return static_cast<EventFlags>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
	}

	inline constexpr EventFlags operator&(EventFlags lhs, EventFlags rhs)
	{
		return static_cast<EventFlags>(static_cast<u32>(lhs) & static_cast<u32>(rhs));
	}

	inline constexpr EventFlags& operator|=(EventFlags& lhs, EventFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline constexpr bool has_event_flags(EventFlags value, EventFlags flags)
	{
		return static_cast<u32>(value & flags) == static_cast<u32>(flags);
	}

	enum class WindowEventKind : u8
	{
		None,
		Shown,
		Hidden,
		Moved,
		Resized,
		FocusGained,
		FocusLost,
		CloseRequested,
	};

	enum class KeyEventKind : u8
	{
		None,
		Pressed,
		Released,
		Repeated,
	};

	enum class PointerEventKind : u8
	{
		None,
		Moved,
		ButtonPressed,
		ButtonReleased,
		Wheel,
		Entered,
		Left,
	};

	struct EventHeader {
		EventTypeId type_id      = 0;
		EventSequence sequence   = 0;
		EventTimestamp timestamp = 0;
		EventFlags flags         = EventFlags::None;
		Identifier source_id     = 0;
		Identifier window_id     = 0;
	};

	struct Event {
		EventHeader header;

		Event() = default;
		explicit Event(const EventHeader& value) : header(value) {}
	};

	struct WindowEvent {
		EventHeader header;
		WindowEventKind kind = WindowEventKind::None;
		Vector2i position    = {0, 0};
		Vector2u size        = {0, 0};
	};

	struct KeyEvent {
		EventHeader header;
		KeyEventKind kind = KeyEventKind::None;
		u32 key_code      = 0;
		u32 scan_code     = 0;
		bool is_repeat    = false;
	};

	struct TextInputEvent {
		EventHeader header;
		char32_t codepoint = U'\0';
		bool is_composing  = false;
	};

	struct PointerEvent {
		EventHeader header;
		PointerEventKind kind    = PointerEventKind::None;
		Identifier pointer_id    = 0;
		u32 button               = 0;
		Vector2f screen_position = {0.f, 0.f};
		Vector2f delta           = {0.f, 0.f};
		Vector2f wheel_delta     = {0.f, 0.f};
	};

	struct EventDispatchResult {
		bool handled               = false;
		bool consumed              = false;
		bool continue_propagation  = true;
		bool emit_gameplay_actions = true;
		EventPhase terminal_phase  = EventPhase::None;

		inline void mark_handled() { handled = true; }

		inline void mark_consumed()
		{
			handled              = true;
			consumed             = true;
			continue_propagation = false;
		}
	};

	struct RoutedEvent : public Event {
		EventPhase phase            = EventPhase::None;
		EventTarget* routing_root   = nullptr;
		EventTarget* target         = nullptr;
		EventTarget* current_target = nullptr;
		void* payload               = nullptr;
		usize payload_size          = 0;
		EventDispatchResult result;
		Vector<EventTarget*> route;

		RoutedEvent() = default;
		explicit RoutedEvent(const EventHeader& value) : Event(value) {}

		inline bool is_handled() const { return result.handled; }
		inline bool is_consumed() const { return result.consumed; }
		inline void mark_handled()
		{
			result.mark_handled();
			header.flags |= EventFlags::Handled;
		}
		inline void mark_consumed()
		{
			result.mark_consumed();
			header.flags |= EventFlags::Handled | EventFlags::Consumed;
		}
	};

	class ENGINE_EXPORT EventListener
	{
	public:
		virtual ~EventListener()                                 = default;
		virtual EventDispatchResult on_event(RoutedEvent& event) = 0;
	};

	class ENGINE_EXPORT EventTarget
	{
	public:
		using ListenerList = Vector<EventListener*>;

	private:
		ListenerList m_preview_listeners;
		ListenerList m_capture_listeners;
		ListenerList m_target_listeners;
		ListenerList m_bubble_listeners;
		ListenerList m_unhandled_listeners;
		ListenerList m_post_listeners;

		ListenerList& listeners_for_phase(EventPhase phase);
		const ListenerList& listeners_for_phase(EventPhase phase) const;

	public:
		virtual ~EventTarget() = default;

		EventTarget& add_listener(EventPhase phase, EventListener* listener);
		EventTarget& remove_listener(EventPhase phase, EventListener* listener);
		const ListenerList& listeners(EventPhase phase) const;

		virtual EventTarget* event_parent() const;
		virtual EventDispatchResult on_preview_event(RoutedEvent& event);
		virtual EventDispatchResult on_capture_event(RoutedEvent& event);
		virtual EventDispatchResult on_target_event(RoutedEvent& event);
		virtual EventDispatchResult on_bubble_event(RoutedEvent& event);
		virtual EventDispatchResult on_unhandled_event(RoutedEvent& event);
		virtual EventDispatchResult on_post_event(RoutedEvent& event);
	};

	class ENGINE_EXPORT EventQueue
	{
	private:
		Vector<Event> m_events;

	public:
		EventQueue& push(const Event& event);
		EventQueue& clear();
		usize size() const;
		bool empty() const;
		const Vector<Event>& events() const;
	};

	class ENGINE_EXPORT DeferredEventQueue
	{
	private:
		Vector<Event> m_messages;

	public:
		DeferredEventQueue& enqueue(const Event& event);
		DeferredEventQueue& clear();
		DeferredEventQueue& drain_to(Vector<Event>& output);
		usize size() const;
		bool empty() const;
		const Vector<Event>& messages() const;
	};

	class ENGINE_EXPORT EventDispatcher
	{
	public:
		using ListenerList = Vector<EventListener*>;

	private:
		Map<EventTypeId, ListenerList> m_listeners;

		static EventDispatchResult merge_result(EventDispatchResult& destination, const EventDispatchResult& source);
		static void finalize_event_result(RoutedEvent& event);
		static bool should_dispatch_phase(const RoutedEvent& event, EventPhase phase);
		Vector<EventTarget*> build_route(EventTarget* target, EventTarget* routing_root) const;
		EventDispatchResult dispatch_listeners(const ListenerList& listeners, RoutedEvent& event) const;
		EventDispatchResult notify_listeners(RoutedEvent& event);
		EventDispatchResult dispatch_single_phase(RoutedEvent& event, EventPhase phase, EventTarget* target);

	public:
		EventDispatcher& add_listener(EventTypeId type_id, EventListener* listener);
		EventDispatcher& remove_listener(EventTypeId type_id, EventListener* listener);
		EventDispatchResult dispatch(RoutedEvent& event);
		EventDispatchResult dispatch_to_target(RoutedEvent& event, EventTarget* target);
	};

	class ENGINE_EXPORT EventSystem : public Singletone<EventSystem, TickableObject>
	{
	public:
		static EventSystem* s_instance;

	private:
		friend class Singletone<EventSystem, TickableObject>;

		EventSequence m_next_sequence = 1;
		EventQueue m_event_queue;
		DeferredEventQueue m_deferred_messages;
		EventDispatcher m_dispatcher;
		EventSystem() = default;

	public:
		EventDispatcher& dispatcher();
		DeferredEventQueue& deferred_messages();
		EventQueue& event_queue();

		EventHeader make_header(EventTypeId type_id, EventFlags flags = EventFlags::None);
		EventDispatchResult route(RoutedEvent& event);
		EventSystem& submit_raw_event(const RawInputEvent& event);
		EventSystem& submit_raw_event_batch(const RawInputEventBatch& batch);
		EventSystem& queue_deferred(const Event& event);

		EventSystem& begin_frame() override;
		EventSystem& update(float dt) override;
		EventSystem& end_frame() override;
	};
}// namespace Trinex::Migration
