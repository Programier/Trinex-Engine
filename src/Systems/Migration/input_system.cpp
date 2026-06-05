#include <Core/math/math.hpp>
#include <Systems/Migration/input_system.hpp>
#include <algorithm>


namespace Trinex::Migration
{
	InputSystem* InputSystem::s_instance = nullptr;

	namespace
	{
		template<typename Enum>
		static constexpr usize enum_index(Enum value)
		{
			return static_cast<usize>(value);
		}

		template<typename Enum>
		static constexpr bool enum_in_range(Enum value, usize capacity)
		{
			return enum_index(value) < capacity;
		}

		static InputDevice default_device(DeviceId device_id, InputDeviceType type, InputUserId user_id)
		{
			InputDevice device;
			device.device_id = device_id;
			device.type      = type;
			device.user_id   = user_id;

			switch (type)
			{
				case InputDeviceType::Keyboard: device.supports_text_input = true; break;

				case InputDeviceType::Mouse:
				case InputDeviceType::Touch: device.supports_pointer = true; break;

				case InputDeviceType::Gamepad: device.supports_gamepad = true; break;

				case InputDeviceType::Undefined:
				case InputDeviceType::Virtual:
				default: break;
			}

			return device;
		}
	}// namespace

	const InputDevice* InputSystem::find_device_by_type(InputDeviceType type, InputUserId user_id) const
	{
		for (const auto& [device_id, device] : m_devices)
		{
			if (device.type == type && device.user_id == user_id && device.is_connected)
			{
				return &device;
			}
		}

		return nullptr;
	}

	const InputDeviceState* InputSystem::find_state_by_type(InputDeviceType type, InputUserId user_id) const
	{
		if (const InputDevice* device = find_device_by_type(type, user_id))
		{
			return device_state(device->device_id);
		}

		return nullptr;
	}

	InputActionValue InputActionValue::boolean(bool value)
	{
		InputActionValue result;
		result.type    = InputActionValueType::Boolean;
		result.value.x = value ? 1.f : 0.f;
		return result;
	}

	InputActionValue InputActionValue::axis_1d(f32 x)
	{
		InputActionValue result;
		result.type    = InputActionValueType::Axis1D;
		result.value.x = x;
		return result;
	}

	InputActionValue InputActionValue::axis_2d(const Vector2f& value)
	{
		InputActionValue result;
		result.type    = InputActionValueType::Axis2D;
		result.value.x = value.x;
		result.value.y = value.y;
		return result;
	}

	InputActionValue InputModifier::apply(const RawInputEvent&, const InputActionValue& value) const
	{
		return value;
	}

	InputActionPhase InputTrigger::evaluate(const RawInputEvent&, const InputActionValue&) const
	{
		return InputActionPhase::Performed;
	}

	InputContextStack& InputContextStack::push(const InputMappingContext* context)
	{
		if (context)
		{
			m_contexts.push_back(context);
			sort_by_priority();
		}

		return *this;
	}

	InputContextStack& InputContextStack::remove(const InputMappingContext* context)
	{
		for (auto it = m_contexts.begin(); it != m_contexts.end(); ++it)
		{
			if (*it == context)
			{
				m_contexts.erase(it);
				break;
			}
		}

		return *this;
	}

	InputContextStack& InputContextStack::clear()
	{
		m_contexts.clear();
		return *this;
	}

	InputContextStack& InputContextStack::sort_by_priority()
	{
		std::sort(m_contexts.begin(), m_contexts.end(), [](const InputMappingContext* lhs, const InputMappingContext* rhs) {
			const i32 left  = lhs ? lhs->priority : 0;
			const i32 right = rhs ? rhs->priority : 0;
			return left > right;
		});

		return *this;
	}

	const Vector<const InputMappingContext*>& InputContextStack::contexts() const
	{
		return m_contexts;
	}

	InputCommandBuffer& InputCommandBuffer::push(const InputCommand& command)
	{
		m_commands.push_back(command);
		return *this;
	}

	InputCommandBuffer& InputCommandBuffer::clear()
	{
		m_commands.clear();
		return *this;
	}

	InputCommandBuffer& InputCommandBuffer::drain_to(Vector<InputCommand>& output)
	{
		for (const InputCommand& command : m_commands)
		{
			output.push_back(command);
		}

		m_commands.clear();
		return *this;
	}

	bool InputCommandBuffer::empty() const
	{
		return m_commands.empty();
	}

	usize InputCommandBuffer::size() const
	{
		return m_commands.size();
	}

	const Vector<InputCommand>& InputCommandBuffer::commands() const
	{
		return m_commands;
	}

	InputDeviceState& InputSystem::find_or_create_state(DeviceId device_id, InputDeviceType type, InputUserId user_id)
	{
		if (m_devices.find(device_id) == m_devices.end())
		{
			m_devices[device_id] = default_device(device_id, type, user_id);
		}

		InputDeviceState& state = m_device_states[device_id];
		state.device_id         = device_id;
		state.type              = type;
		state.user_id           = user_id;
		return state;
	}

	InputSystem& InputSystem::begin_frame()
	{
		for (auto& [device_id, state] : m_device_states)
		{
			if (state.type == InputDeviceType::Mouse || state.type == InputDeviceType::Touch)
			{
				// Pointer delta queries are per-frame values, so they must be cleared before new input is applied.
				state.pointer.delta       = {0.f, 0.f};
				state.pointer.wheel_delta = {0.f, 0.f};
			}
		}

		m_pending_action_events.clear();
		m_pending_raw_events.first_sequence = 0;
		m_pending_raw_events.last_sequence  = 0;
		m_pending_raw_events.events.clear();
		return *this;
	}

	InputSystem& InputSystem::update(float dt)
	{
		return *this;
	}

	InputSystem& InputSystem::end_frame()
	{
		return *this;
	}

	InputSystem& InputSystem::register_device(const InputDevice& device)
	{
		m_devices[device.device_id] = device;
		find_or_create_state(device.device_id, device.type, device.user_id);
		return *this;
	}

	InputSystem& InputSystem::unregister_device(DeviceId device_id)
	{
		m_devices.erase(device_id);
		m_device_states.erase(device_id);
		return *this;
	}

	InputSystem& InputSystem::submit_raw_event(const RawInputEvent& event)
	{
		if (m_pending_raw_events.events.empty())
		{
			m_pending_raw_events.first_sequence = event.header.sequence;
		}

		m_pending_raw_events.last_sequence = event.header.sequence;
		m_pending_raw_events.events.push_back(event);
		return *this;
	}

	InputSystem& InputSystem::submit_raw_event_batch(const RawInputEventBatch& batch)
	{
		if (batch.events.empty())
		{
			return *this;
		}

		if (m_pending_raw_events.events.empty())
		{
			m_pending_raw_events.first_sequence = batch.first_sequence;
		}

		m_pending_raw_events.last_sequence = batch.last_sequence;

		for (const RawInputEvent& event : batch.events)
		{
			m_pending_raw_events.events.push_back(event);
		}

		return *this;
	}

	InputSystem& InputSystem::update_device_state(const RawInputEventBatch& batch)
	{
		for (const RawInputEvent& event : batch.events)
		{
			InputDeviceState& state = find_or_create_state(event.device_id, event.device_type, event.user_id);

			switch (event.type)
			{
				case RawInputEventType::Key:
				{
					const KeyCode key_code   = static_cast<KeyCode::Enum>(event.key.key_code);
					const ScanCode scan_code = static_cast<ScanCode::Enum>(event.key.scan_code);
					const bool is_pressed    = event.key.kind != KeyEventKind::Released;

					if (enum_in_range(key_code, key_capacity()))
					{
						state.keyboard.keys[enum_index(key_code)] = is_pressed;
					}

					if (enum_in_range(scan_code, scan_code_capacity()))
					{
						state.keyboard.scan_codes[enum_index(scan_code)] = is_pressed;
					}
					break;
				}

				case RawInputEventType::Pointer:
					state.pointer.pointer_id      = static_cast<PointerId>(event.pointer.pointer_id);
					state.pointer.screen_position = event.pointer.screen_position;
					state.pointer.delta += event.pointer.delta;
					state.pointer.wheel_delta += event.pointer.wheel_delta;

					if (event.pointer.button != 0)
					{
						const MouseButton button = static_cast<MouseButton::Enum>(event.pointer.button);

						if (enum_in_range(button, mouse_button_capacity()))
						{
							state.pointer.buttons[enum_index(button)] = event.pointer.kind != PointerEventKind::ButtonReleased;
						}
					}
					break;

				case RawInputEventType::Gamepad:
				{
					if (event.gamepad.kind == GamepadEventKind::AxisMotion)
					{
						if (enum_in_range(event.gamepad.axis, gamepad_axis_capacity()))
						{
							state.gamepad.axes[enum_index(event.gamepad.axis)] = event.gamepad.value;
						}
					}
					else
					{
						if (enum_in_range(event.gamepad.button, gamepad_button_capacity()))
						{
							state.gamepad.buttons[enum_index(event.gamepad.button)] =
							        event.gamepad.kind == GamepadEventKind::ButtonPressed;
						}
					}
					break;
				}

				case RawInputEventType::DeviceChange:
				{
					InputDevice& device = m_devices[event.device_id];
					device.device_id    = event.device_id;
					device.type         = event.device_type;
					device.user_id      = event.user_id;

					if (event.device_change.kind == DeviceChangeKind::Added)
					{
						device = default_device(event.device_id, event.device_type, event.user_id);
					}
					else if (event.device_change.kind == DeviceChangeKind::Remapped)
					{
						InputDevice updated  = default_device(event.device_id, event.device_type, event.user_id);
						updated.is_connected = device.is_connected;
						device               = updated;
					}

					if (event.device_change.kind == DeviceChangeKind::Removed)
					{
						device.is_connected = false;
						state               = InputDeviceState();
						state.device_id     = event.device_id;
						state.type          = event.device_type;
						state.user_id       = event.user_id;
					}
					else
					{
						device.is_connected = true;
					}
					break;
				}

				case RawInputEventType::Window:
				case RawInputEventType::TextInput:
				case RawInputEventType::Undefined:
				default: break;
			}
		}

		return *this;
	}

	InputSystem& InputSystem::build_command_buffer()
	{
		m_command_buffer.clear();

		for (const InputActionEvent& event : m_pending_action_events)
		{
			InputCommand command;
			command.sequence     = event.header.sequence;
			command.user_id      = event.user_id;
			command.action_event = event;
			m_command_buffer.push(command);
		}

		return *this;
	}

	InputContextStack& InputSystem::context_stack(InputUserId user_id)
	{
		return m_user_contexts[user_id];
	}

	InputCommandBuffer& InputSystem::command_buffer()
	{
		return m_command_buffer;
	}

	const InputDevice* InputSystem::device(DeviceId device_id) const
	{
		if (auto it = m_devices.find(device_id); it != m_devices.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const InputDeviceState* InputSystem::device_state(DeviceId device_id) const
	{
		if (auto it = m_device_states.find(device_id); it != m_device_states.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const InputDevice* InputSystem::keyboard_device(InputUserId user_id) const
	{
		return find_device_by_type(InputDeviceType::Keyboard, user_id);
	}

	const InputDevice* InputSystem::pointer_device(InputUserId user_id) const
	{
		if (const InputDevice* device = find_device_by_type(InputDeviceType::Mouse, user_id))
		{
			return device;
		}

		return find_device_by_type(InputDeviceType::Touch, user_id);
	}

	const InputDevice* InputSystem::gamepad_device(InputUserId user_id) const
	{
		return find_device_by_type(InputDeviceType::Gamepad, user_id);
	}

	const InputDeviceState* InputSystem::keyboard_state(DeviceId device_id) const
	{
		const InputDeviceState* state = device_state(device_id);
		return state && state->type == InputDeviceType::Keyboard ? state : nullptr;
	}

	const InputDeviceState* InputSystem::pointer_state(DeviceId device_id) const
	{
		const InputDeviceState* state = device_state(device_id);
		return state && (state->type == InputDeviceType::Mouse || state->type == InputDeviceType::Touch) ? state : nullptr;
	}

	const InputDeviceState* InputSystem::gamepad_state(DeviceId device_id) const
	{
		const InputDeviceState* state = device_state(device_id);
		return state && state->type == InputDeviceType::Gamepad ? state : nullptr;
	}

	const InputDeviceState* InputSystem::keyboard_state_for_user(InputUserId user_id) const
	{
		return find_state_by_type(InputDeviceType::Keyboard, user_id);
	}

	const InputDeviceState* InputSystem::pointer_state_for_user(InputUserId user_id) const
	{
		if (const InputDeviceState* state = find_state_by_type(InputDeviceType::Mouse, user_id))
		{
			return state;
		}

		return find_state_by_type(InputDeviceType::Touch, user_id);
	}

	const InputDeviceState* InputSystem::gamepad_state_for_user(InputUserId user_id) const
	{
		return find_state_by_type(InputDeviceType::Gamepad, user_id);
	}

	bool InputSystem::is_key_pressed(KeyCode key_code, DeviceId device_id) const
	{
		if (const InputDeviceState* state = keyboard_state(device_id))
		{
			return enum_in_range(key_code, key_capacity()) ? state->keyboard.keys[enum_index(key_code)] : false;
		}

		return false;
	}

	bool InputSystem::is_key_released(KeyCode key_code, DeviceId device_id) const
	{
		return !is_key_pressed(key_code, device_id);
	}

	bool InputSystem::is_scan_code_pressed(ScanCode scan_code, DeviceId device_id) const
	{
		if (const InputDeviceState* state = keyboard_state(device_id))
		{
			return enum_in_range(scan_code, scan_code_capacity()) ? state->keyboard.scan_codes[enum_index(scan_code)] : false;
		}

		return false;
	}

	bool InputSystem::is_scan_code_released(ScanCode scan_code, DeviceId device_id) const
	{
		return !is_scan_code_pressed(scan_code, device_id);
	}

	bool InputSystem::is_key_pressed_for_user(KeyCode key_code, InputUserId user_id) const
	{
		if (const InputDeviceState* state = keyboard_state_for_user(user_id))
		{
			return enum_in_range(key_code, key_capacity()) ? state->keyboard.keys[enum_index(key_code)] : false;
		}

		return false;
	}

	bool InputSystem::is_key_released_for_user(KeyCode key_code, InputUserId user_id) const
	{
		return !is_key_pressed_for_user(key_code, user_id);
	}

	bool InputSystem::is_scan_code_pressed_for_user(ScanCode scan_code, InputUserId user_id) const
	{
		if (const InputDeviceState* state = keyboard_state_for_user(user_id))
		{
			return enum_in_range(scan_code, scan_code_capacity()) ? state->keyboard.scan_codes[enum_index(scan_code)] : false;
		}

		return false;
	}

	bool InputSystem::is_scan_code_released_for_user(ScanCode scan_code, InputUserId user_id) const
	{
		return !is_scan_code_pressed_for_user(scan_code, user_id);
	}

	bool InputSystem::is_mouse_button_pressed(MouseButton button, DeviceId device_id) const
	{
		if (const InputDeviceState* state = pointer_state(device_id))
		{
			return enum_in_range(button, mouse_button_capacity()) ? state->pointer.buttons[enum_index(button)] : false;
		}

		return false;
	}

	bool InputSystem::is_mouse_button_released(MouseButton button, DeviceId device_id) const
	{
		return !is_mouse_button_pressed(button, device_id);
	}

	bool InputSystem::is_mouse_button_pressed_for_user(MouseButton button, InputUserId user_id) const
	{
		if (const InputDeviceState* state = pointer_state_for_user(user_id))
		{
			return enum_in_range(button, mouse_button_capacity()) ? state->pointer.buttons[enum_index(button)] : false;
		}

		return false;
	}

	bool InputSystem::is_mouse_button_released_for_user(MouseButton button, InputUserId user_id) const
	{
		return !is_mouse_button_pressed_for_user(button, user_id);
	}

	Vector2f InputSystem::pointer_screen_position(DeviceId device_id) const
	{
		if (const InputDeviceState* state = pointer_state(device_id))
		{
			return state->pointer.screen_position;
		}

		return {0.f, 0.f};
	}

	Vector2f InputSystem::pointer_delta(DeviceId device_id) const
	{
		if (const InputDeviceState* state = pointer_state(device_id))
		{
			return state->pointer.delta;
		}

		return {0.f, 0.f};
	}

	Vector2f InputSystem::pointer_wheel_delta(DeviceId device_id) const
	{
		if (const InputDeviceState* state = pointer_state(device_id))
		{
			return state->pointer.wheel_delta;
		}

		return {0.f, 0.f};
	}

	Vector2f InputSystem::pointer_screen_position_for_user(InputUserId user_id) const
	{
		if (const InputDeviceState* state = pointer_state_for_user(user_id))
		{
			return state->pointer.screen_position;
		}

		return {0.f, 0.f};
	}

	Vector2f InputSystem::pointer_delta_for_user(InputUserId user_id) const
	{
		if (const InputDeviceState* state = pointer_state_for_user(user_id))
		{
			return state->pointer.delta;
		}

		return {0.f, 0.f};
	}

	Vector2f InputSystem::pointer_wheel_delta_for_user(InputUserId user_id) const
	{
		if (const InputDeviceState* state = pointer_state_for_user(user_id))
		{
			return state->pointer.wheel_delta;
		}

		return {0.f, 0.f};
	}

	bool InputSystem::has_pointer(DeviceId device_id) const
	{
		return pointer_state(device_id) != nullptr;
	}

	bool InputSystem::has_pointer_for_user(InputUserId user_id) const
	{
		return pointer_state_for_user(user_id) != nullptr;
	}

	bool InputSystem::is_gamepad_button_pressed(GamepadButton button, DeviceId device_id) const
	{
		if (const InputDeviceState* state = gamepad_state(device_id))
		{
			return enum_in_range(button, gamepad_button_capacity()) ? state->gamepad.buttons[enum_index(button)] : false;
		}

		return false;
	}

	bool InputSystem::is_gamepad_button_released(GamepadButton button, DeviceId device_id) const
	{
		return !is_gamepad_button_pressed(button, device_id);
	}

	bool InputSystem::is_gamepad_button_pressed_for_user(GamepadButton button, InputUserId user_id) const
	{
		if (const InputDeviceState* state = gamepad_state_for_user(user_id))
		{
			return enum_in_range(button, gamepad_button_capacity()) ? state->gamepad.buttons[enum_index(button)] : false;
		}

		return false;
	}

	bool InputSystem::is_gamepad_button_released_for_user(GamepadButton button, InputUserId user_id) const
	{
		return !is_gamepad_button_pressed_for_user(button, user_id);
	}

	f32 InputSystem::gamepad_axis_value(GamepadAxis axis, DeviceId device_id, f32 dead_zone) const
	{
		if (const InputDeviceState* state = gamepad_state(device_id))
		{
			if (enum_in_range(axis, gamepad_axis_capacity()))
			{
				const f32 value = state->gamepad.axes[enum_index(axis)];
				return (Math::abs(value) >= Math::abs(dead_zone)) ? value : 0.f;
			}
		}

		return 0.f;
	}

	f32 InputSystem::gamepad_axis_value_for_user(GamepadAxis axis, InputUserId user_id, f32 dead_zone) const
	{
		if (const InputDeviceState* state = gamepad_state_for_user(user_id))
		{
			if (enum_in_range(axis, gamepad_axis_capacity()))
			{
				const f32 value = state->gamepad.axes[enum_index(axis)];
				return (Math::abs(value) >= Math::abs(dead_zone)) ? value : 0.f;
			}
		}

		return 0.f;
	}

	bool InputSystem::has_gamepad(DeviceId device_id) const
	{
		return gamepad_state(device_id) != nullptr;
	}

	bool InputSystem::has_gamepad_for_user(InputUserId user_id) const
	{
		return gamepad_state_for_user(user_id) != nullptr;
	}

	bool InputSystem::supports_text_input(DeviceId device_id) const
	{
		if (const InputDevice* found = device(device_id))
		{
			return found->supports_text_input;
		}

		return false;
	}

	bool InputSystem::supports_text_input_for_user(InputUserId user_id) const
	{
		if (const InputDevice* found = keyboard_device(user_id))
		{
			return found->supports_text_input;
		}

		return false;
	}

	const Map<DeviceId, InputDeviceState>& InputSystem::device_states() const
	{
		return m_device_states;
	}

	const Map<DeviceId, InputDevice>& InputSystem::devices() const
	{
		return m_devices;
	}

	const Vector<InputActionEvent>& InputSystem::pending_action_events() const
	{
		return m_pending_action_events;
	}

	const RawInputEventBatch& InputSystem::pending_raw_events() const
	{
		return m_pending_raw_events;
	}
}// namespace Trinex::Migration
