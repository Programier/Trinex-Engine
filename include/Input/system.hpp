#pragma once
#include <Core/etl/delegate.hpp>
#include <Core/etl/vector.hpp>
#include <Core/ref_counted.hpp>
#include <Input/actions.hpp>
#include <Input/device.hpp>
#include <Input/types.hpp>

namespace Trinex
{
	class ENGINE_EXPORT InputSystem : public RefCounted
	{
	public:
		// Return true to consume an event and stop its propagation.
		using Listener = Delegate<bool(const InputEvent&)>;

	private:
		static InputSystem* s_instance;

		struct ListenerBinding {
			InputDevice* device;
			EventType type;
			EventCode code;
			Listener listener;
			InputListenerHandle handle;
			mutable bool enabled = true;
		};

	private:
		Vector<Ref<InputDevice>> m_devices;
		Vector<ListenerBinding> m_listeners;
		Vector<ListenerBinding> m_pending_listeners;
		struct MappingState {
			Ref<InputContext> context;
			usize mapping_index  = 0;
			u64 context_revision = 0;
			InputDevice* device  = nullptr;
			InputEvent last_event;
			Vector<InputTriggerState> triggers;
		};

		struct ActionListenerBinding {
			InputActionListener listener;
			InputActionListenerHandle handle;
		};

		struct ActiveInputContext {
			Ref<InputContext> context;
			i32 priority               = 0;
			bool blocks_lower_priority = false;
		};

		Vector<ActiveInputContext> m_contexts;
		Vector<MappingState> m_mapping_states;
		Vector<InputActionState> m_action_states;
		Vector<ActionListenerBinding> m_action_listeners;
		u32 m_dispatch_depth = 0;

		static bool matches(const ListenerBinding& binding, const InputEvent& event);
		void flush_pending_listeners();
		void sort_contexts();
		MappingState& mapping_state(const Ref<InputContext>& context, usize mapping_index, InputDevice* device);
		InputActionState& action_state(const Ref<InputAction>& action);
		void emit(MappingState& mapping, const InputMapping& source, InputActionPhase phase, const InputValue& value);
		bool dispatch_actions(const InputEvent& event);
		void cancel_device(InputDevice* device);
		static bool matches(const InputControl& control, const InputEvent& event);
		static InputValue event_value(const InputEvent& event);
		static InputActionPhase evaluate(const InputMapping& mapping, const InputEvent& event, const InputValue& value,
		                                 MappingState& state);

	public:
		InputSystem();
		~InputSystem() override;

		static InputSystem* instance();

		InputSystem& add_device(const Ref<InputDevice>& device);
		InputSystem& remove_device(const InputDevice* device);
		const Vector<Ref<InputDevice>>& devices() const { return m_devices; }
		InputDevice* device(u32 id);
		const InputDevice* device(u32 id) const;
		const InputEvent* state(const InputDevice* device, EventCode code) const;
		const InputEvent* state(u32 device_id, EventCode code) const;
		bool is_pressed(const InputDevice* device, EventCode code) const;
		bool is_pressed(u32 device_id, EventCode code) const;
		bool is_pressed(EventCode code) const;
		InputValue value(const InputDevice* device, EventCode code) const;
		InputValue value(u32 device_id, EventCode code) const;
		InputValue value(EventCode code) const;

		InputListenerHandle add_listener(InputDevice* device, EventType type, EventCode code, const Listener& listener);
		InputSystem& remove_listener(InputListenerHandle handle);
		InputSystem& add_context(const Ref<InputContext>& context, i32 priority = 0, bool blocks_lower_priority = false);
		InputSystem& remove_context(const InputContext* context);
		InputSystem& clear_contexts();
		InputActionListenerHandle add_action_listener(const InputActionListener& listener);
		InputSystem& remove_action_listener(InputActionListenerHandle handle);
		const InputActionState* action_state(const InputAction* action) const;
		InputSystem& begin_frame();
		InputSystem& update(f32 dt);

		// Returns true when a listener consumes the event.
		bool dispatch(const InputEvent& event);
	};
}// namespace Trinex
