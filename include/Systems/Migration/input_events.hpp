#pragma once

#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Systems/Migration/event_system.hpp>
#include <Systems/Migration/input_codes.hpp>

namespace Trinex::Migration
{
	struct RawInputEventType {
		enum Enum : u8
		{
			Undefined,
			Window,
			Key,
			TextInput,
			Pointer,
			Gamepad,
			DeviceChange,
		};

		trinex_enum_struct(RawInputEventType);
	};

	struct DeviceChangeEvent {
		EventHeader header;
		DeviceChangeKind kind       = DeviceChangeKind::Undefined;
		DeviceId device_id          = 0;
		InputDeviceType device_type = InputDeviceType::Undefined;
		InputUserId user_id         = 0;
	};

	struct GamepadEvent {
		EventHeader header;
		GamepadEventKind kind = GamepadEventKind::Undefined;
		GamepadButton button  = GamepadButton::Undefined;
		GamepadAxis axis      = GamepadAxis::Undefined;
		f32 value             = 0.f;
	};

	struct InputDevice {
		DeviceId device_id       = 0;
		InputDeviceType type     = InputDeviceType::Undefined;
		InputUserId user_id      = 0;
		bool is_connected        = true;
		bool supports_text_input = false;
		bool supports_pointer    = false;
		bool supports_gamepad    = false;
	};

	struct KeyboardDeviceState {
		bool keys[static_cast<usize>(KeyCode::Count)]        = {};
		bool scan_codes[static_cast<usize>(ScanCode::Count)] = {};
	};

	struct PointerDeviceState {
		PointerId pointer_id                                 = 0;
		Vector2f screen_position                             = {0.f, 0.f};
		Vector2f delta                                       = {0.f, 0.f};
		Vector2f wheel_delta                                 = {0.f, 0.f};
		bool buttons[static_cast<usize>(MouseButton::Count)] = {};
	};

	struct GamepadDeviceState {
		bool buttons[static_cast<usize>(GamepadButton::Count)] = {};
		f32 axes[static_cast<usize>(GamepadAxis::Count)]       = {};
	};

	struct InputDeviceState {
		DeviceId device_id   = 0;
		InputDeviceType type = InputDeviceType::Undefined;
		InputUserId user_id  = 0;
		KeyboardDeviceState keyboard;
		PointerDeviceState pointer;
		GamepadDeviceState gamepad;
	};

	struct RawInputEvent {
		RawInputEventType type      = RawInputEventType::Undefined;
		DeviceId device_id          = 0;
		InputUserId user_id         = 0;
		InputDeviceType device_type = InputDeviceType::Undefined;
		EventHeader header;
		WindowEvent window;
		KeyEvent key;
		TextInputEvent text_input;
		PointerEvent pointer;
		GamepadEvent gamepad;
		DeviceChangeEvent device_change;
	};

	struct RawInputEventBatch {
		EventSequence first_sequence = 0;
		EventSequence last_sequence  = 0;
		Vector<RawInputEvent> events;
	};
}// namespace Trinex::Migration
