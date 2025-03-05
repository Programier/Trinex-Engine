#pragma once
#include <Core/enums.hpp>
#include <Core/game_controller.hpp>
#include <Core/keyboard.hpp>
#include <Core/mouse.hpp>

namespace Engine
{
	enum class EventType : byte
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
			Engine::Keyboard::Key key;
		};

		union Mouse
		{
			struct MouseWheelEvent {
				float x;
				float y;
			} wheel;

			struct MouseMotionEvent {
				float x;
				float y;
				float xrel;
				float yrel;
			} motion;

			struct MouseButtonEvent {
				Engine::Mouse::Button button;
				float x;
				float y;
			} button;
		};

		struct Gamepad {
			Identifier id;

			struct AxisMotionEvent {
				Engine::GameController::Axis axis;
				float value;
			};

			union
			{
				AxisMotionEvent axis_motion;
			};
		};

		union TouchScreen
		{
			struct FingerEvent {
				Index index;
				float x;
				float y;
			} finger;

			struct FingerMotionEvent {
				Index index;
				float x;
				float y;
				float xrel;
				float yrel;
			} finger_motion;
		};

		struct TextInput {
			static constexpr uint32_t max_tex_len = 31;
			char text[max_tex_len + 1];
		};

		union
		{
			Display display;
			Window window;
			Keyboard keyboard;
			Mouse mouse;
			Gamepad gamepad;
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
}// namespace Engine
