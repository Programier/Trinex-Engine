#pragma once
#include <Core/etl/handle.hpp>
#include <Core/etl/storage.hpp>
#include <Core/math/vector.hpp>
#include <Core/types/name.hpp>

namespace Trinex
{
	using InputListenerHandle = Handle<class InputListener>;

	struct InputValueType {
		enum Enum : u8
		{
			Undefined,
			Boolean,
			Integer,
			Unsigned,
			Axis1D,
			Axis2D,
			Axis3D,
			Text,
		};

		trinex_enum_struct(InputValueType);
	};

	struct InputValue {
		InputValueType type = InputValueType::Undefined;
		Vector3f axes       = {0.f, 0.f, 0.f};

		static InputValue boolean(bool value) { return {InputValueType::Boolean, {value ? 1.f : 0.f, 0.f, 0.f}}; }

		static InputValue axis_1d(f32 x) { return {InputValueType::Axis1D, {x, 0.f, 0.f}}; }
		static InputValue axis_2d(const Vector2f& value) { return {InputValueType::Axis2D, {value.x, value.y, 0.f}}; }
		static InputValue axis_3d(const Vector3f& value) { return {InputValueType::Axis3D, value}; }

		bool is_active() const { return axes.x != 0.f || axes.y != 0.f || axes.z != 0.f; }
	};

	struct ENGINE_EXPORT InputDeviceType : private Name {
		using Name::c_str;
		using Name::hash;
		using Name::id;
		using Name::is_valid;
		using Name::Name;
		using Name::to_string;
		using Name::operator bool;

		static const InputDeviceType Undefined;

		static const InputDeviceType Keyboard;
		static const InputDeviceType Mouse;
		static const InputDeviceType Gamepad;
		static const InputDeviceType Touch;
		static const InputDeviceType Pen;
		static const InputDeviceType MotionController;
		static const InputDeviceType RacingWheel;
		static const InputDeviceType FlightStick;

		inline bool operator==(const InputDeviceType& other) const { return Name::operator==(other); }
		inline bool operator!=(const InputDeviceType& other) const { return Name::operator!=(other); }
	};

	struct ENGINE_EXPORT EventType : private Name {
		using Name::c_str;
		using Name::hash;
		using Name::id;
		using Name::is_valid;
		using Name::Name;
		using Name::to_string;
		using Name::operator bool;

		static const EventType Undefined;

		// Input
		static const EventType Press;
		static const EventType Release;
		static const EventType Repeat;
		static const EventType Axis;
		static const EventType TextEditing;
		static const EventType TextInput;

		// Device
		static const EventType DeviceAdded;
		static const EventType DeviceRemoved;

		// Keyboard
		static const EventType KeymapChanged;

		// Display
		static const EventType DisplayOrientationChanged;
		static const EventType DisplayContentScaleChanged;

		// Window
		static const EventType WindowShown;
		static const EventType WindowHidden;
		static const EventType WindowMoved;
		static const EventType WindowResized;
		static const EventType WindowPixelSizeChanged;
		static const EventType WindowMinimized;
		static const EventType WindowMaximized;
		static const EventType WindowRestored;
		static const EventType WindowMouseEnter;
		static const EventType WindowMouseLeave;
		static const EventType WindowFocusGained;
		static const EventType WindowFocusLost;
		static const EventType WindowCloseRequested;
		static const EventType WindowDisplayChanged;
		static const EventType WindowDisplayScaleChanged;
		static const EventType WindowEnterFullscreen;
		static const EventType WindowLeaveFullscreen;
		static const EventType WindowDestroyed;

		// Application
		static const EventType Quit;
		static const EventType Terminating;
		static const EventType LowMemory;
		static const EventType WillEnterBackground;
		static const EventType DidEnterBackground;
		static const EventType WillEnterForeground;
		static const EventType DidEnterForeground;
		static const EventType LocaleChanged;
		static const EventType SystemThemeChanged;

		// Clipboard
		static const EventType ClipboardUpdate;

		// Drag & Drop
		static const EventType DropFile;
		static const EventType DropText;
		static const EventType DropBegin;
		static const EventType DropComplete;
		static const EventType DropPosition;

		inline bool operator==(const EventType& other) const { return Name::operator==(other); }
		inline bool operator!=(const EventType& other) const { return Name::operator!=(other); }
	};

	struct ENGINE_EXPORT EventCode : private Name {

	public:
		using Name::c_str;
		using Name::hash;
		using Name::id;
		using Name::is_valid;
		using Name::Name;
		using Name::to_string;
		using Name::operator bool;

		static const EventCode Undefined;

		// Keyboard
		static const EventCode KeyA;
		static const EventCode KeyB;
		static const EventCode KeyC;
		static const EventCode KeyD;
		static const EventCode KeyE;
		static const EventCode KeyF;
		static const EventCode KeyG;
		static const EventCode KeyH;
		static const EventCode KeyI;
		static const EventCode KeyJ;
		static const EventCode KeyK;
		static const EventCode KeyL;
		static const EventCode KeyM;
		static const EventCode KeyN;
		static const EventCode KeyO;
		static const EventCode KeyP;
		static const EventCode KeyQ;
		static const EventCode KeyR;
		static const EventCode KeyS;
		static const EventCode KeyT;
		static const EventCode KeyU;
		static const EventCode KeyV;
		static const EventCode KeyW;
		static const EventCode KeyX;
		static const EventCode KeyY;
		static const EventCode KeyZ;

		static const EventCode Key0;
		static const EventCode Key1;
		static const EventCode Key2;
		static const EventCode Key3;
		static const EventCode Key4;
		static const EventCode Key5;
		static const EventCode Key6;
		static const EventCode Key7;
		static const EventCode Key8;
		static const EventCode Key9;

		static const EventCode KeyF1;
		static const EventCode KeyF2;
		static const EventCode KeyF3;
		static const EventCode KeyF4;
		static const EventCode KeyF5;
		static const EventCode KeyF6;
		static const EventCode KeyF7;
		static const EventCode KeyF8;
		static const EventCode KeyF9;
		static const EventCode KeyF10;
		static const EventCode KeyF11;
		static const EventCode KeyF12;

		static const EventCode KeyEscape;
		static const EventCode KeyTab;
		static const EventCode KeyCapsLock;
		static const EventCode KeySpace;
		static const EventCode KeyEnter;
		static const EventCode KeyBackspace;

		static const EventCode KeyInsert;
		static const EventCode KeyDelete;
		static const EventCode KeyHome;
		static const EventCode KeyEnd;
		static const EventCode KeyPageUp;
		static const EventCode KeyPageDown;

		static const EventCode KeyLeft;
		static const EventCode KeyRight;
		static const EventCode KeyUp;
		static const EventCode KeyDown;

		static const EventCode KeyLeftShift;
		static const EventCode KeyRightShift;
		static const EventCode KeyLeftControl;
		static const EventCode KeyRightControl;
		static const EventCode KeyLeftAlt;
		static const EventCode KeyRightAlt;
		static const EventCode KeyLeftSuper;
		static const EventCode KeyRightSuper;

		static const EventCode KeyMinus;
		static const EventCode KeyEqual;
		static const EventCode KeyLeftBracket;
		static const EventCode KeyRightBracket;
		static const EventCode KeyBackslash;
		static const EventCode KeySemicolon;
		static const EventCode KeyApostrophe;
		static const EventCode KeyGrave;
		static const EventCode KeyComma;
		static const EventCode KeyPeriod;
		static const EventCode KeySlash;

		static const EventCode KeyPrintScreen;
		static const EventCode KeyScrollLock;
		static const EventCode KeyPause;

		// Numpad
		static const EventCode KeyNumpad0;
		static const EventCode KeyNumpad1;
		static const EventCode KeyNumpad2;
		static const EventCode KeyNumpad3;
		static const EventCode KeyNumpad4;
		static const EventCode KeyNumpad5;
		static const EventCode KeyNumpad6;
		static const EventCode KeyNumpad7;
		static const EventCode KeyNumpad8;
		static const EventCode KeyNumpad9;

		static const EventCode KeyNumpadDecimal;
		static const EventCode KeyNumpadDivide;
		static const EventCode KeyNumpadMultiply;
		static const EventCode KeyNumpadSubtract;
		static const EventCode KeyNumpadAdd;
		static const EventCode KeyNumpadEnter;
		static const EventCode KeyNumpadEqual;
		static const EventCode KeyNumLock;

		// Mouse buttons
		static const EventCode MouseLeft;
		static const EventCode MouseRight;
		static const EventCode MouseMiddle;
		static const EventCode MouseBack;
		static const EventCode MouseForward;

		// Mouse axes
		static const EventCode MouseX;
		static const EventCode MouseY;
		static const EventCode MouseDeltaX;
		static const EventCode MouseDeltaY;
		static const EventCode MouseWheelX;
		static const EventCode MouseWheelY;
		static const EventCode Text;

		// Gamepad buttons
		static const EventCode GamepadFaceBottom;
		static const EventCode GamepadFaceRight;
		static const EventCode GamepadFaceLeft;
		static const EventCode GamepadFaceTop;

		static const EventCode GamepadLeftShoulder;
		static const EventCode GamepadRightShoulder;

		static const EventCode GamepadLeftStickButton;
		static const EventCode GamepadRightStickButton;

		static const EventCode GamepadDPadUp;
		static const EventCode GamepadDPadDown;
		static const EventCode GamepadDPadLeft;
		static const EventCode GamepadDPadRight;

		static const EventCode GamepadStart;
		static const EventCode GamepadBack;
		static const EventCode GamepadGuide;

		// Gamepad axes
		static const EventCode GamepadLeftX;
		static const EventCode GamepadLeftY;
		static const EventCode GamepadRightX;
		static const EventCode GamepadRightY;
		static const EventCode GamepadLeftTrigger;
		static const EventCode GamepadRightTrigger;

		inline bool operator==(const EventCode& other) const { return Name::operator==(other); }
		inline bool operator!=(const EventCode& other) const { return Name::operator!=(other); }
	};

	class InputDevice;

	struct InputEvent {
		InputDevice* device     = nullptr;
		u64 window_id           = 0;
		EventType type          = EventType::Undefined;
		EventCode code          = EventCode::Undefined;
		InputValue value        = {};
		Storage<32, 16> storage = {};

		template<typename T>
		T& payload()
		{
			return storage.as<T>();
		}

		template<typename T>
		const T& payload() const
		{
			return storage.as<T>();
		}
	};

}// namespace Trinex
