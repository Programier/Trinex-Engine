#pragma once

#include <Core/engine_types.hpp>
#include <Systems/Migration/input_codes.def>

namespace Trinex::Migration
{
	using DeviceId    = u64;
	using InputUserId = u32;
	using PointerId   = u32;

	struct KeyCode {
		enum Enum : u16
		{
#define TRINEX_MIGRATION_KEY_ENUM(name, value) name = value,
			TRINEX_MIGRATION_KEY_CODE_LIST(TRINEX_MIGRATION_KEY_ENUM)
#undef TRINEX_MIGRATION_KEY_ENUM
			        Count = 512,
		};

		trinex_enum_struct(KeyCode);
	};

	struct ScanCode {
		enum Enum : u16
		{
#define TRINEX_MIGRATION_SCAN_ENUM(name, value) name = value,
			TRINEX_MIGRATION_KEY_CODE_LIST(TRINEX_MIGRATION_SCAN_ENUM)
#undef TRINEX_MIGRATION_SCAN_ENUM
			        Count = 512,
		};

		trinex_enum_struct(ScanCode);
	};

	struct MouseButton {
		enum Enum : u8
		{
			Undefined = 0,
			Left,
			Right,
			Middle,
			X1,
			X2,
			TouchPrimary,
			PenPrimary,
			PenSecondary,
			Count,
		};

		trinex_enum_struct(MouseButton);
	};

	struct GamepadButton {
		enum Enum : u8
		{
			Undefined = 0,
			FaceBottom,
			FaceRight,
			FaceLeft,
			FaceTop,
			LeftShoulder,
			RightShoulder,
			LeftTrigger,
			RightTrigger,
			LeftStick,
			RightStick,
			DPadUp,
			DPadDown,
			DPadLeft,
			DPadRight,
			Guide,
			Start,
			Select,
			Misc1,
			Paddle1,
			Paddle2,
			Paddle3,
			Paddle4,
			Touchpad,
			Count,
		};

		trinex_enum_struct(GamepadButton);
	};

	struct GamepadAxis {
		enum Enum : u8
		{
			Undefined = 0,
			LeftX,
			LeftY,
			RightX,
			RightY,
			LeftTrigger,
			RightTrigger,
			Count,
		};

		trinex_enum_struct(GamepadAxis);
	};

	struct DeviceChangeKind {
		enum Enum : u8
		{
			Undefined,
			Added,
			Removed,
			Remapped,
		};

		trinex_enum_struct(DeviceChangeKind);
	};

	struct GamepadEventKind {
		enum Enum : u8
		{
			Undefined,
			ButtonPressed,
			ButtonReleased,
			AxisMotion,
		};

		trinex_enum_struct(GamepadEventKind);
	};

	struct InputDeviceType {
		enum Enum : u8
		{
			Undefined,
			Keyboard,
			Mouse,
			Touch,
			Gamepad,
			Virtual,
		};

		trinex_enum_struct(InputDeviceType);
	};
}// namespace Trinex::Migration
