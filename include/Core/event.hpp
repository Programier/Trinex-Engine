#pragma once
#include <Core/enums.hpp>
#include <Core/keyboard.hpp>

namespace Trinex
{
	enum class EventType : u8
	{
		Undefined,

		// Application events
		Quit,
		AppTerminating,
		AppLowMemory,
		AppPause,
		AppResume,

		TextInput,

		// Display events
		DisplayAdded,
		DisplayRemoved,
		DisplayOrientationChanged,

		// Window events
		WindowShown,
		WindowHidden,
		WindowMoved,
		WindowResized,
		WindowMinimized,
		WindowMaximized,

		WindowRestored,
		WindowFocusGained,
		WindowFocusLost,
		WindowClose,

		// Keyboard events
		KeyDown,
		KeyUp,

		// Mouse events
		MouseMotion,
		MouseButtonUp,
		MouseButtonDown,
		MouseWheel,

		// Game controller events
		ControllerAxisMotion,
		ControllerButtonUp,
		ControllerButtonDown,
		ControllerDeviceAdded,
		ControllerDeviceRemoved,
		ControllerDeviceRemapped,
		ControllerTouchPadDown,
		ControllerTouchPadMotion,
		ControllerTouchPadUp,
		ControllerSensorUpdate,

		// Touch events
		FingerDown,
		FingerUp,
		FingerMotion,

		// Drag and drop events
		DropFile,
		DropText,
		DropBegin,
		DropComplete,

		COUNT
	};

	struct ENGINE_EXPORT Event {
		struct Display {
			Orientation orientation;
		};

		struct Window {
			union
			{
				float x;
				float width;
			};

			union
			{
				float y;
				float height;
			};
		};

		struct Keyboard {
			Trinex::Keyboard::Key key;
		};

		union TouchScreen
		{
			struct FingerEvent {
				usize index;
				float x;
				float y;
			} finger;

			struct FingerMotionEvent {
				usize index;
				float x;
				float y;
				float xrel;
				float yrel;
			} finger_motion;
		};

		struct TextInput {
			static constexpr u32 max_tex_len = 31;
			char text[max_tex_len + 1];
		};

		union
		{
			Display display;
			Window window;
			Keyboard keyboard;
			TouchScreen touchscreen;
			TextInput text_input;
		};

		Identifier window_id = 0;
		EventType type       = EventType::Undefined;

		Event();
		Event(const Event&)            = default;
		Event(Event&&)                 = default;
		Event& operator=(const Event&) = default;
		Event& operator=(Event&&)      = default;
	};
}// namespace Trinex
