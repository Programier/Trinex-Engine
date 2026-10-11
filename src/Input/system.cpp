#include <Core/etl/algorithm.hpp>
#include <Input/device.hpp>
#include <Input/system.hpp>

namespace Trinex
{
	InputSystem* InputSystem::s_instance = nullptr;

	InputSystem::InputSystem()
	{
		trinex_assert(s_instance == nullptr);
		s_instance = this;
	}

	InputSystem::~InputSystem()
	{
		if (s_instance == this)
			s_instance = nullptr;
	}

	InputSystem* InputSystem::instance()
	{
		return s_instance;
	}

	InputSystem& InputSystem::add_device(const Ref<InputDevice>& device)
	{
		if (device && etl::find(m_devices.begin(), m_devices.end(), device) == m_devices.end())
		{
			m_devices.emplace_back(device);
		}

		return *this;
	}

	InputSystem& InputSystem::remove_device(const InputDevice* device)
	{
		cancel_device(const_cast<InputDevice*>(device));

		auto it = etl::find(m_devices.begin(), m_devices.end(), device);

		if (it != m_devices.end())
		{
			m_devices.erase_unordered(it);
		}

		for (const ListenerBinding& binding : m_listeners)
		{
			if (binding.device == device)
			{
				binding.enabled = false;
			}
		}

		for (auto it = m_pending_listeners.begin(); it != m_pending_listeners.end();)
		{
			if (it->device == device)
			{
				it = m_pending_listeners.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (!m_dispatch_depth)
		{
			flush_pending_listeners();
		}

		return *this;
	}

	InputDevice* InputSystem::device(u32 id)
	{
		return const_cast<InputDevice*>(static_cast<const InputSystem*>(this)->device(id));
	}

	const InputDevice* InputSystem::device(u32 id) const
	{
		for (const Ref<InputDevice>& device : m_devices)
		{
			if (device && device->id() == id)
			{
				return device.value();
			}
		}

		return nullptr;
	}

	const InputEvent* InputSystem::state(const InputDevice* device, EventCode code) const
	{
		if (!device || !code)
		{
			return nullptr;
		}

		u32 count                = 0;
		const InputEvent* events = device->state(count);
		for (u32 index = 0; events && index < count; ++index)
		{
			if (events[index].code == code)
			{
				return &events[index];
			}
		}

		return nullptr;
	}

	const InputEvent* InputSystem::state(u32 device_id, EventCode code) const
	{
		return state(device(device_id), code);
	}

	bool InputSystem::is_pressed(const InputDevice* device, EventCode code) const
	{
		const InputEvent* event = state(device, code);
		return event && (event->type == EventType::Press || event->type == EventType::Repeat ||
		                 (event->value.type == InputValueType::Boolean && event->value.is_active()));
	}

	bool InputSystem::is_pressed(u32 device_id, EventCode code) const
	{
		return is_pressed(device(device_id), code);
	}

	bool InputSystem::is_pressed(EventCode code) const
	{
		for (const Ref<InputDevice>& device : m_devices)
		{
			if (is_pressed(device.value(), code))
				return true;
		}

		return false;
	}

	InputValue InputSystem::value(const InputDevice* device, EventCode code) const
	{
		if (const InputEvent* event = state(device, code))
		{
			return event_value(*event);
		}

		return {};
	}

	InputValue InputSystem::value(u32 device_id, EventCode code) const
	{
		return value(device(device_id), code);
	}

	InputValue InputSystem::value(EventCode code) const
	{
		for (const Ref<InputDevice>& device : m_devices)
		{
			if (InputValue result = value(device.value(), code); result.type != InputValueType::Undefined)
				return result;
		}

		return {};
	}

	InputListenerHandle InputSystem::add_listener(InputDevice* device, EventType type, EventCode code, const Listener& listener)
	{
		InputListenerHandle handle = InputListenerHandle::allocate();
		ListenerBinding binding    = {device, type, code, listener, handle};

		if (m_dispatch_depth)
		{
			m_pending_listeners.emplace_back(etl::move(binding));
		}
		else
		{
			m_listeners.emplace_back(etl::move(binding));
		}

		return handle;
	}

	InputSystem& InputSystem::remove_listener(InputListenerHandle handle)
	{
		if (!handle)
			return *this;

		for (const ListenerBinding& binding : m_listeners)
		{
			if (binding.handle.value() == handle.value())
			{
				binding.enabled = false;
				break;
			}
		}

		for (auto it = m_pending_listeners.begin(); it != m_pending_listeners.end(); ++it)
		{
			if (it->handle.value() == handle.value())
			{
				m_pending_listeners.erase(it);
				break;
			}
		}

		if (!m_dispatch_depth)
		{
			flush_pending_listeners();
		}

		return *this;
	}

	InputSystem& InputSystem::begin_frame()
	{
		for (InputActionState& state : m_action_states)
		{
			state.triggered_this_frame = false;
		}
		return *this;
	}

	InputSystem& InputSystem::update(f32 dt)
	{
		for (auto it = m_mapping_states.begin(); it != m_mapping_states.end();)
		{
			MappingState& mapping = *it;
			if (!mapping.context || mapping.context_revision != mapping.context->revision() ||
			    mapping.mapping_index >= mapping.context->mappings().size())
			{
				it = m_mapping_states.erase(it);
				continue;
			}

			const InputMapping& source = mapping.context->mappings()[mapping.mapping_index];
			for (usize index = 0; index < source.triggers.size(); ++index)
			{
				const Ref<InputTrigger>& trigger = source.triggers[index];
				if (trigger)
				{
					const InputActionPhase phase = trigger->update(dt, mapping.triggers[index]);
					if (phase != InputActionPhase::None)
					{
						emit(mapping, source, phase, mapping.triggers[index].value);
					}
				}
			}

			++it;
		}
		return *this;
	}

	bool InputSystem::dispatch(const InputEvent& event)
	{
		if (dispatch_actions(event))
		{
			return true;
		}

		++m_dispatch_depth;

		for (const ListenerBinding& binding : m_listeners)
		{
			if (binding.enabled && matches(binding, event) && binding.listener && binding.listener(event))
			{
				--m_dispatch_depth;

				if (!m_dispatch_depth)
				{
					flush_pending_listeners();
				}

				return true;
			}
		}

		--m_dispatch_depth;

		if (!m_dispatch_depth)
		{
			flush_pending_listeners();
		}

		return false;
	}

	bool InputSystem::matches(const ListenerBinding& binding, const InputEvent& event)
	{
		return (!binding.device || binding.device == event.device) && (!binding.type || binding.type == event.type) &&
		       (!binding.code || binding.code == event.code);
	}

	void InputSystem::flush_pending_listeners()
	{
		for (auto it = m_listeners.begin(); it != m_listeners.end();)
		{
			if (!it->enabled)
			{
				it = m_listeners.erase(it);
			}
			else
			{
				++it;
			}
		}

		for (ListenerBinding& binding : m_pending_listeners)
		{
			if (binding.enabled)
			{
				m_listeners.emplace_back(etl::move(binding));
			}
		}

		m_pending_listeners.clear();
	}

	void InputSystem::sort_contexts()
	{
		etl::sort(m_contexts.begin(), m_contexts.end(),
		          [](const ActiveInputContext& lhs, const ActiveInputContext& rhs) { return lhs.priority > rhs.priority; });
	}

	InputSystem::MappingState& InputSystem::mapping_state(const Ref<InputContext>& context, usize mapping_index,
	                                                      InputDevice* device)
	{
		for (MappingState& state : m_mapping_states)
		{
			if (state.context == context && state.mapping_index == mapping_index && state.device == device &&
			    state.context_revision == context->revision())
			{
				return state;
			}
		}

		MappingState& state    = m_mapping_states.emplace_back();
		state.context          = context;
		state.mapping_index    = mapping_index;
		state.context_revision = context->revision();
		state.device           = device;
		state.triggers.resize(context->mappings()[mapping_index].triggers.size());
		return state;
	}

	InputActionState& InputSystem::action_state(const Ref<InputAction>& action)
	{
		for (InputActionState& state : m_action_states)
		{
			if (state.action == action)
			{
				return state;
			}
		}

		InputActionState& state = m_action_states.emplace_back();
		state.action            = action;
		return state;
	}

	void InputSystem::emit(MappingState& mapping, const InputMapping& source, InputActionPhase phase, const InputValue& value)
	{
		InputActionState& state = action_state(source.action);
		state.value             = value;
		state.phase             = phase;
		state.triggered_this_frame |= phase == InputActionPhase::Triggered;

		InputActionEvent event = {source.action, value, phase, mapping.device, mapping.context.value()};
		for (const ActionListenerBinding& binding : m_action_listeners)
		{
			if (binding.listener && binding.listener(event))
			{
				break;
			}
		}
	}

	bool InputSystem::matches(const InputControl& control, const InputEvent& event)
	{
		bool matching_type = !control.device_type;

		if (!matching_type && event.device)
		{
			for (u32 index = 0; index < event.device->types(); ++index)
			{
				if (event.device->type(index) == control.device_type)
				{
					matching_type = true;
					break;
				}
			}
		}

		return matching_type && (!control.device_id || (event.device && event.device->id() == control.device_id)) &&
		       (!control.type || control.type == event.type) && (!control.code || control.code == event.code);
	}

	InputValue InputSystem::event_value(const InputEvent& event)
	{
		if (event.value.type != InputValueType::Undefined)
		{
			return event.value;
		}

		if (event.type == EventType::Press || event.type == EventType::Repeat)
		{
			return InputValue::boolean(true);
		}

		return event.type == EventType::Release ? InputValue::boolean(false) : InputValue();
	}

	InputActionPhase InputSystem::evaluate(const InputMapping& mapping, const InputEvent& event, const InputValue& value,
	                                       MappingState& state)
	{
		if (mapping.triggers.empty())
		{
			return value.is_active() ? InputActionPhase::Triggered : InputActionPhase::Completed;
		}

		InputActionPhase result = InputActionPhase::Triggered;
		for (usize index = 0; index < mapping.triggers.size(); ++index)
		{
			const Ref<InputTrigger>& trigger = mapping.triggers[index];
			if (!trigger)
			{
				return InputActionPhase::None;
			}

			const InputActionPhase phase = trigger->evaluate(event, value, state.triggers[index]);
			if (phase == InputActionPhase::None)
			{
				return InputActionPhase::None;
			}

			if (phase != InputActionPhase::Triggered)
			{
				result = phase;
			}
		}

		return result;
	}

	bool InputSystem::dispatch_actions(const InputEvent& event)
	{
		for (const ActiveInputContext& active : m_contexts)
		{
			if (!active.context || !active.context->enabled())
			{
				continue;
			}

			bool matched = false;
			for (usize index = 0; index < active.context->mappings().size(); ++index)
			{
				const InputMapping& mapping = active.context->mappings()[index];
				if (!matches(mapping.control, event))
				{
					continue;
				}

				InputValue value = event_value(event);
				for (const Ref<InputModifier>& modifier : mapping.modifiers)
				{
					if (modifier)
					{
						value = modifier->apply(event, value);
					}
				}

				MappingState& state          = mapping_state(active.context, index, event.device);
				state.last_event             = event;
				const InputActionPhase phase = evaluate(mapping, event, value, state);
				if (phase == InputActionPhase::None)
				{
					continue;
				}

				matched = true;
				emit(state, mapping, phase, value);
				if (mapping.consume)
				{
					return true;
				}
				if (mapping.block_lower_priority)
				{
					return false;
				}
			}

			if (matched && active.blocks_lower_priority)
			{
				break;
			}
		}

		return false;
	}

	void InputSystem::cancel_device(InputDevice* device)
	{
		for (auto it = m_mapping_states.begin(); it != m_mapping_states.end();)
		{
			MappingState& mapping = *it;
			if (mapping.device != device)
			{
				++it;
				continue;
			}

			if (mapping.context && mapping.context_revision == mapping.context->revision() &&
			    mapping.mapping_index < mapping.context->mappings().size())
			{
				const InputMapping& source = mapping.context->mappings()[mapping.mapping_index];
				emit(mapping, source, InputActionPhase::Canceled, {});
			}

			it = m_mapping_states.erase(it);
		}
	}

	InputSystem& InputSystem::add_context(const Ref<InputContext>& context, i32 priority, bool blocks_lower_priority)
	{
		if (!context)
		{
			return *this;
		}

		for (ActiveInputContext& active : m_contexts)
		{
			if (active.context == context)
			{
				active.priority              = priority;
				active.blocks_lower_priority = blocks_lower_priority;
				sort_contexts();
				return *this;
			}
		}

		m_contexts.emplace_back(context, priority, blocks_lower_priority);
		sort_contexts();
		return *this;
	}

	InputSystem& InputSystem::remove_context(const InputContext* context)
	{
		for (auto it = m_contexts.begin(); it != m_contexts.end(); ++it)
		{
			if (it->context == context)
			{
				m_contexts.erase(it);
				break;
			}
		}

		for (auto it = m_mapping_states.begin(); it != m_mapping_states.end();)
		{
			it = it->context == context ? m_mapping_states.erase(it) : it + 1;
		}

		return *this;
	}

	InputSystem& InputSystem::clear_contexts()
	{
		m_contexts.clear();
		m_mapping_states.clear();
		return *this;
	}

	InputActionListenerHandle InputSystem::add_action_listener(const InputActionListener& listener)
	{
		InputActionListenerHandle handle = InputActionListenerHandle::allocate();
		m_action_listeners.emplace_back(listener, handle);
		return handle;
	}

	InputSystem& InputSystem::remove_action_listener(InputActionListenerHandle handle)
	{
		for (auto it = m_action_listeners.begin(); it != m_action_listeners.end(); ++it)
		{
			if (it->handle.value() == handle.value())
			{
				m_action_listeners.erase(it);
				break;
			}
		}

		return *this;
	}

	const InputActionState* InputSystem::action_state(const InputAction* action) const
	{
		for (const InputActionState& state : m_action_states)
		{
			if (state.action == action)
			{
				return &state;
			}
		}

		return nullptr;
	}

}// namespace Trinex
