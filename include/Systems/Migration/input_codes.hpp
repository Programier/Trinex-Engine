#pragma once

#include <Core/engine_types.hpp>
#include <Systems/Migration/input_codes.def>

namespace Trinex::Migration
{
	using DeviceId    = u64;
	using InputUserId = u32;
	using PointerId   = u32;

	enum class KeyCode : u16
	{
#define TRINEX_MIGRATION_KEY_ENUM(name, value) name = value,
		TRINEX_MIGRATION_KEY_CODE_LIST(TRINEX_MIGRATION_KEY_ENUM)
#undef TRINEX_MIGRATION_KEY_ENUM
		        Count = 512,
	};

	enum class ScanCode : u16
	{
#define TRINEX_MIGRATION_SCAN_ENUM(name, value) name = value,
		TRINEX_MIGRATION_KEY_CODE_LIST(TRINEX_MIGRATION_SCAN_ENUM)
#undef TRINEX_MIGRATION_SCAN_ENUM
		        Count = 512,
	};

	enum class MouseButton : u8
	{
		None = 0,
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

	enum class GamepadButton : u8
	{
		None = 0,
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

	enum class GamepadAxis : u8
	{
		None = 0,
		LeftX,
		LeftY,
		RightX,
		RightY,
		LeftTrigger,
		RightTrigger,
		Count,
	};

	enum class DeviceChangeKind : u8
	{
		None,
		Added,
		Removed,
		Remapped,
	};

	enum class GamepadEventKind : u8
	{
		None,
		ButtonPressed,
		ButtonReleased,
		AxisMotion,
	};

	enum class InputDeviceType : u8
	{
		Unknown,
		Keyboard,
		Mouse,
		Touch,
		Gamepad,
		Virtual,
	};
}// namespace Trinex::Migration
