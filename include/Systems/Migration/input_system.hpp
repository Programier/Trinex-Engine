#pragma once

#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Core/tickable.hpp>
#include <Systems/Migration/event_system.hpp>
#include <Systems/Migration/input_codes.hpp>
#include <Systems/Migration/input_events.hpp>

namespace Trinex::Migration
{
	struct InputRoutingResult;

	using InputActionId = u64;

	enum class InputActionValueType : u8
	{
		None,
		Boolean,
		Axis1D,
		Axis2D,
		Axis3D,
	};

	enum class InputActionPhase : u8
	{
		Waiting,
		Started,
		Ongoing,
		Performed,
		Canceled,
	};

	enum class InputControlKind : u8
	{
		None,
		Key,
		ScanCode,
		MouseButton,
		PointerDelta,
		PointerPosition,
		MouseWheel,
		GamepadButton,
		GamepadAxis,
		TextInput,
	};

	struct InputActionValue {
		InputActionValueType type = InputActionValueType::None;
		Vector4f value            = {0.f, 0.f, 0.f, 0.f};

		static InputActionValue boolean(bool value);
		static InputActionValue axis_1d(f32 x);
		static InputActionValue axis_2d(const Vector2f& value);
	};

	struct InputAction {
		InputActionId id                = 0;
		const char* debug_name          = nullptr;
		InputActionValueType value_type = InputActionValueType::None;
		bool trigger_while_paused       = false;
		bool allow_replay_recording     = true;
		bool allow_network_prediction   = true;
	};

	struct InputControlPath {
		InputControlKind kind        = InputControlKind::None;
		KeyCode key_code             = KeyCode::Unknown;
		ScanCode scan_code           = ScanCode::Unknown;
		MouseButton mouse_button     = MouseButton::None;
		GamepadButton gamepad_button = GamepadButton::None;
		GamepadAxis gamepad_axis     = GamepadAxis::None;
		i32 axis_sign                = 0;
	};

	class ENGINE_EXPORT InputModifier
	{
	public:
		virtual ~InputModifier() = default;
		virtual InputActionValue apply(const RawInputEvent& event, const InputActionValue& value) const;
	};

	class ENGINE_EXPORT InputTrigger
	{
	public:
		virtual ~InputTrigger() = default;
		virtual InputActionPhase evaluate(const RawInputEvent& event, const InputActionValue& value) const;
	};

	struct InputBinding {
		InputActionId action_id = 0;
		InputControlPath control;
		Vector<const InputModifier*> modifiers;
		Vector<const InputTrigger*> triggers;
		bool consume_routed_event          = false;
		bool block_lower_priority_contexts = false;
	};

	struct InputMappingContext {
		Identifier context_id      = 0;
		const char* debug_name     = nullptr;
		i32 priority               = 0;
		bool enabled               = true;
		bool blocks_lower_priority = false;
		Vector<InputBinding> bindings;
	};

	class ENGINE_EXPORT InputContextStack
	{
	private:
		Vector<const InputMappingContext*> m_contexts;

	public:
		InputContextStack& push(const InputMappingContext* context);
		InputContextStack& remove(const InputMappingContext* context);
		InputContextStack& clear();
		InputContextStack& sort_by_priority();
		const Vector<const InputMappingContext*>& contexts() const;
	};

	struct InputActionEvent {
		EventHeader header;
		InputActionId action_id = 0;
		InputUserId user_id     = 0;
		DeviceId device_id      = 0;
		InputActionPhase phase  = InputActionPhase::Waiting;
		InputActionValue value;
		bool was_blocked_by_routing = false;
	};

	struct InputCommand {
		EventSequence sequence = 0;
		InputUserId user_id    = 0;
		InputActionEvent action_event;
	};

	class ENGINE_EXPORT InputCommandBuffer
	{
	private:
		Vector<InputCommand> m_commands;

	public:
		InputCommandBuffer& push(const InputCommand& command);
		InputCommandBuffer& clear();
		InputCommandBuffer& drain_to(Vector<InputCommand>& output);
		bool empty() const;
		usize size() const;
		const Vector<InputCommand>& commands() const;
	};

	class ENGINE_EXPORT InputSystem : public Singletone<InputSystem, TickableObject>
	{
	public:
		static InputSystem* s_instance;

	private:
		friend class Singletone<InputSystem, TickableObject>;

		Map<DeviceId, InputDevice> m_devices;
		Map<DeviceId, InputDeviceState> m_device_states;
		Map<InputUserId, InputContextStack> m_user_contexts;
		Vector<InputActionEvent> m_pending_action_events;
		RawInputEventBatch m_pending_raw_events;
		InputCommandBuffer m_command_buffer;

		InputDeviceState& find_or_create_state(DeviceId device_id, InputDeviceType type, InputUserId user_id);
		const InputDevice* find_device_by_type(InputDeviceType type, InputUserId user_id) const;
		const InputDeviceState* find_state_by_type(InputDeviceType type, InputUserId user_id) const;
		InputSystem() = default;

	public:
		InputSystem& begin_frame() override;
		InputSystem& update(float dt) override;
		InputSystem& end_frame() override;
		InputSystem& register_device(const InputDevice& device);
		InputSystem& unregister_device(DeviceId device_id);
		InputSystem& submit_raw_event(const RawInputEvent& event);
		InputSystem& submit_raw_event_batch(const RawInputEventBatch& batch);
		InputSystem& update_device_state(const RawInputEventBatch& batch);
		InputSystem& build_command_buffer();

		InputContextStack& context_stack(InputUserId user_id);
		InputCommandBuffer& command_buffer();
		const InputDevice* device(DeviceId device_id) const;
		const InputDeviceState* device_state(DeviceId device_id) const;
		const InputDevice* keyboard_device(InputUserId user_id = 0) const;
		const InputDevice* pointer_device(InputUserId user_id = 0) const;
		const InputDevice* gamepad_device(InputUserId user_id = 0) const;
		const InputDeviceState* keyboard_state(DeviceId device_id) const;
		const InputDeviceState* pointer_state(DeviceId device_id) const;
		const InputDeviceState* gamepad_state(DeviceId device_id) const;
		const InputDeviceState* keyboard_state_for_user(InputUserId user_id = 0) const;
		const InputDeviceState* pointer_state_for_user(InputUserId user_id = 0) const;
		const InputDeviceState* gamepad_state_for_user(InputUserId user_id = 0) const;

		bool is_key_pressed(KeyCode key_code, DeviceId device_id) const;
		bool is_key_released(KeyCode key_code, DeviceId device_id) const;
		bool is_scan_code_pressed(ScanCode scan_code, DeviceId device_id) const;
		bool is_scan_code_released(ScanCode scan_code, DeviceId device_id) const;
		bool is_key_pressed_for_user(KeyCode key_code, InputUserId user_id = 0) const;
		bool is_key_released_for_user(KeyCode key_code, InputUserId user_id = 0) const;
		bool is_scan_code_pressed_for_user(ScanCode scan_code, InputUserId user_id = 0) const;
		bool is_scan_code_released_for_user(ScanCode scan_code, InputUserId user_id = 0) const;

		bool is_mouse_button_pressed(MouseButton button, DeviceId device_id) const;
		bool is_mouse_button_released(MouseButton button, DeviceId device_id) const;
		bool is_mouse_button_pressed_for_user(MouseButton button, InputUserId user_id = 0) const;
		bool is_mouse_button_released_for_user(MouseButton button, InputUserId user_id = 0) const;
		Vector2f pointer_screen_position(DeviceId device_id) const;
		Vector2f pointer_delta(DeviceId device_id) const;
		Vector2f pointer_wheel_delta(DeviceId device_id) const;
		Vector2f pointer_screen_position_for_user(InputUserId user_id = 0) const;
		Vector2f pointer_delta_for_user(InputUserId user_id = 0) const;
		Vector2f pointer_wheel_delta_for_user(InputUserId user_id = 0) const;
		bool has_pointer(DeviceId device_id) const;
		bool has_pointer_for_user(InputUserId user_id = 0) const;

		bool is_gamepad_button_pressed(GamepadButton button, DeviceId device_id) const;
		bool is_gamepad_button_released(GamepadButton button, DeviceId device_id) const;
		bool is_gamepad_button_pressed_for_user(GamepadButton button, InputUserId user_id = 0) const;
		bool is_gamepad_button_released_for_user(GamepadButton button, InputUserId user_id = 0) const;
		f32 gamepad_axis_value(GamepadAxis axis, DeviceId device_id, f32 dead_zone = 0.f) const;
		f32 gamepad_axis_value_for_user(GamepadAxis axis, InputUserId user_id = 0, f32 dead_zone = 0.f) const;
		bool has_gamepad(DeviceId device_id) const;
		bool has_gamepad_for_user(InputUserId user_id = 0) const;

		bool supports_text_input(DeviceId device_id) const;
		bool supports_text_input_for_user(InputUserId user_id = 0) const;

		const Map<DeviceId, InputDeviceState>& device_states() const;
		const Map<DeviceId, InputDevice>& devices() const;
		const Vector<InputActionEvent>& pending_action_events() const;
		const RawInputEventBatch& pending_raw_events() const;

		static constexpr usize key_capacity() { return static_cast<usize>(KeyCode::Count); }
		static constexpr usize scan_code_capacity() { return static_cast<usize>(ScanCode::Count); }
		static constexpr usize mouse_button_capacity() { return static_cast<usize>(MouseButton::Count); }
		static constexpr usize gamepad_button_capacity() { return static_cast<usize>(GamepadButton::Count); }
		static constexpr usize gamepad_axis_capacity() { return static_cast<usize>(GamepadAxis::Count); }
	};
}// namespace Trinex::Migration
