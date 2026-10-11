#include <Input/types.hpp>

namespace Trinex
{
	const InputDeviceType InputDeviceType::Undefined        = InputDeviceType();
	const InputDeviceType InputDeviceType::Keyboard         = "Keyboard";
	const InputDeviceType InputDeviceType::Mouse            = "Mouse";
	const InputDeviceType InputDeviceType::Gamepad          = "Gamepad";
	const InputDeviceType InputDeviceType::Touch            = "Touch";
	const InputDeviceType InputDeviceType::Pen              = "Pen";
	const InputDeviceType InputDeviceType::MotionController = "MotionController";
	const InputDeviceType InputDeviceType::RacingWheel      = "RacingWheel";
	const InputDeviceType InputDeviceType::FlightStick      = "FlightStick";

	const EventType EventType::Undefined   = EventType();
	const EventType EventType::Press       = "Press";
	const EventType EventType::Release     = "Release";
	const EventType EventType::Repeat      = "Repeat";
	const EventType EventType::Axis        = "Axis";
	const EventType EventType::TextEditing = "TextEditing";
	const EventType EventType::TextInput   = "TextInput";

	// Device
	const EventType EventType::DeviceAdded   = "DeviceAdded";
	const EventType EventType::DeviceRemoved = "DeviceRemoved";

	// Keyboard
	const EventType EventType::KeymapChanged = "KeymapChanged";

	// Display
	const EventType EventType::DisplayOrientationChanged  = "DisplayOrientationChanged";
	const EventType EventType::DisplayContentScaleChanged = "DisplayContentScaleChanged";

	// Window
	const EventType EventType::WindowShown               = "WindowShown";
	const EventType EventType::WindowHidden              = "WindowHidden";
	const EventType EventType::WindowMoved               = "WindowMoved";
	const EventType EventType::WindowResized             = "WindowResized";
	const EventType EventType::WindowPixelSizeChanged    = "WindowPixelSizeChanged";
	const EventType EventType::WindowMinimized           = "WindowMinimized";
	const EventType EventType::WindowMaximized           = "WindowMaximized";
	const EventType EventType::WindowRestored            = "WindowRestored";
	const EventType EventType::WindowMouseEnter          = "WindowMouseEnter";
	const EventType EventType::WindowMouseLeave          = "WindowMouseLeave";
	const EventType EventType::WindowFocusGained         = "WindowFocusGained";
	const EventType EventType::WindowFocusLost           = "WindowFocusLost";
	const EventType EventType::WindowCloseRequested      = "WindowCloseRequested";
	const EventType EventType::WindowDisplayChanged      = "WindowDisplayChanged";
	const EventType EventType::WindowDisplayScaleChanged = "WindowDisplayScaleChanged";
	const EventType EventType::WindowEnterFullscreen     = "WindowEnterFullscreen";
	const EventType EventType::WindowLeaveFullscreen     = "WindowLeaveFullscreen";
	const EventType EventType::WindowDestroyed           = "WindowDestroyed";

	// Application
	const EventType EventType::Quit                = "Quit";
	const EventType EventType::Terminating         = "Terminating";
	const EventType EventType::LowMemory           = "LowMemory";
	const EventType EventType::WillEnterBackground = "WillEnterBackground";
	const EventType EventType::DidEnterBackground  = "DidEnterBackground";
	const EventType EventType::WillEnterForeground = "WillEnterForeground";
	const EventType EventType::DidEnterForeground  = "DidEnterForeground";
	const EventType EventType::LocaleChanged       = "LocaleChanged";
	const EventType EventType::SystemThemeChanged  = "SystemThemeChanged";

	// Clipboard
	const EventType EventType::ClipboardUpdate = "ClipboardUpdate";

	// Drag & Drop
	const EventType EventType::DropFile     = "DropFile";
	const EventType EventType::DropText     = "DropText";
	const EventType EventType::DropBegin    = "DropBegin";
	const EventType EventType::DropComplete = "DropComplete";
	const EventType EventType::DropPosition = "DropPosition";

	// Keyboard
	const EventCode EventCode::Undefined = EventCode();

	const EventCode EventCode::KeyA = "KeyA";
	const EventCode EventCode::KeyB = "KeyB";
	const EventCode EventCode::KeyC = "KeyC";
	const EventCode EventCode::KeyD = "KeyD";
	const EventCode EventCode::KeyE = "KeyE";
	const EventCode EventCode::KeyF = "KeyF";
	const EventCode EventCode::KeyG = "KeyG";
	const EventCode EventCode::KeyH = "KeyH";
	const EventCode EventCode::KeyI = "KeyI";
	const EventCode EventCode::KeyJ = "KeyJ";
	const EventCode EventCode::KeyK = "KeyK";
	const EventCode EventCode::KeyL = "KeyL";
	const EventCode EventCode::KeyM = "KeyM";
	const EventCode EventCode::KeyN = "KeyN";
	const EventCode EventCode::KeyO = "KeyO";
	const EventCode EventCode::KeyP = "KeyP";
	const EventCode EventCode::KeyQ = "KeyQ";
	const EventCode EventCode::KeyR = "KeyR";
	const EventCode EventCode::KeyS = "KeyS";
	const EventCode EventCode::KeyT = "KeyT";
	const EventCode EventCode::KeyU = "KeyU";
	const EventCode EventCode::KeyV = "KeyV";
	const EventCode EventCode::KeyW = "KeyW";
	const EventCode EventCode::KeyX = "KeyX";
	const EventCode EventCode::KeyY = "KeyY";
	const EventCode EventCode::KeyZ = "KeyZ";

	const EventCode EventCode::Key0 = "Key0";
	const EventCode EventCode::Key1 = "Key1";
	const EventCode EventCode::Key2 = "Key2";
	const EventCode EventCode::Key3 = "Key3";
	const EventCode EventCode::Key4 = "Key4";
	const EventCode EventCode::Key5 = "Key5";
	const EventCode EventCode::Key6 = "Key6";
	const EventCode EventCode::Key7 = "Key7";
	const EventCode EventCode::Key8 = "Key8";
	const EventCode EventCode::Key9 = "Key9";

	const EventCode EventCode::KeyF1  = "KeyF1";
	const EventCode EventCode::KeyF2  = "KeyF2";
	const EventCode EventCode::KeyF3  = "KeyF3";
	const EventCode EventCode::KeyF4  = "KeyF4";
	const EventCode EventCode::KeyF5  = "KeyF5";
	const EventCode EventCode::KeyF6  = "KeyF6";
	const EventCode EventCode::KeyF7  = "KeyF7";
	const EventCode EventCode::KeyF8  = "KeyF8";
	const EventCode EventCode::KeyF9  = "KeyF9";
	const EventCode EventCode::KeyF10 = "KeyF10";
	const EventCode EventCode::KeyF11 = "KeyF11";
	const EventCode EventCode::KeyF12 = "KeyF12";

	const EventCode EventCode::KeyEscape    = "KeyEscape";
	const EventCode EventCode::KeyTab       = "KeyTab";
	const EventCode EventCode::KeyCapsLock  = "KeyCapsLock";
	const EventCode EventCode::KeySpace     = "KeySpace";
	const EventCode EventCode::KeyEnter     = "KeyEnter";
	const EventCode EventCode::KeyBackspace = "KeyBackspace";

	const EventCode EventCode::KeyInsert   = "KeyInsert";
	const EventCode EventCode::KeyDelete   = "KeyDelete";
	const EventCode EventCode::KeyHome     = "KeyHome";
	const EventCode EventCode::KeyEnd      = "KeyEnd";
	const EventCode EventCode::KeyPageUp   = "KeyPageUp";
	const EventCode EventCode::KeyPageDown = "KeyPageDown";

	const EventCode EventCode::KeyLeft  = "KeyLeft";
	const EventCode EventCode::KeyRight = "KeyRight";
	const EventCode EventCode::KeyUp    = "KeyUp";
	const EventCode EventCode::KeyDown  = "KeyDown";

	const EventCode EventCode::KeyLeftShift    = "KeyLeftShift";
	const EventCode EventCode::KeyRightShift   = "KeyRightShift";
	const EventCode EventCode::KeyLeftControl  = "KeyLeftControl";
	const EventCode EventCode::KeyRightControl = "KeyRightControl";
	const EventCode EventCode::KeyLeftAlt      = "KeyLeftAlt";
	const EventCode EventCode::KeyRightAlt     = "KeyRightAlt";
	const EventCode EventCode::KeyLeftSuper    = "KeyLeftSuper";
	const EventCode EventCode::KeyRightSuper   = "KeyRightSuper";

	const EventCode EventCode::KeyMinus        = "KeyMinus";
	const EventCode EventCode::KeyEqual        = "KeyEqual";
	const EventCode EventCode::KeyLeftBracket  = "KeyLeftBracket";
	const EventCode EventCode::KeyRightBracket = "KeyRightBracket";
	const EventCode EventCode::KeyBackslash    = "KeyBackslash";
	const EventCode EventCode::KeySemicolon    = "KeySemicolon";
	const EventCode EventCode::KeyApostrophe   = "KeyApostrophe";
	const EventCode EventCode::KeyGrave        = "KeyGrave";
	const EventCode EventCode::KeyComma        = "KeyComma";
	const EventCode EventCode::KeyPeriod       = "KeyPeriod";
	const EventCode EventCode::KeySlash        = "KeySlash";

	const EventCode EventCode::KeyPrintScreen = "KeyPrintScreen";
	const EventCode EventCode::KeyScrollLock  = "KeyScrollLock";
	const EventCode EventCode::KeyPause       = "KeyPause";

	// Numpad
	const EventCode EventCode::KeyNumpad0 = "KeyNumpad0";
	const EventCode EventCode::KeyNumpad1 = "KeyNumpad1";
	const EventCode EventCode::KeyNumpad2 = "KeyNumpad2";
	const EventCode EventCode::KeyNumpad3 = "KeyNumpad3";
	const EventCode EventCode::KeyNumpad4 = "KeyNumpad4";
	const EventCode EventCode::KeyNumpad5 = "KeyNumpad5";
	const EventCode EventCode::KeyNumpad6 = "KeyNumpad6";
	const EventCode EventCode::KeyNumpad7 = "KeyNumpad7";
	const EventCode EventCode::KeyNumpad8 = "KeyNumpad8";
	const EventCode EventCode::KeyNumpad9 = "KeyNumpad9";

	const EventCode EventCode::KeyNumpadDecimal  = "KeyNumpadDecimal";
	const EventCode EventCode::KeyNumpadDivide   = "KeyNumpadDivide";
	const EventCode EventCode::KeyNumpadMultiply = "KeyNumpadMultiply";
	const EventCode EventCode::KeyNumpadSubtract = "KeyNumpadSubtract";
	const EventCode EventCode::KeyNumpadAdd      = "KeyNumpadAdd";
	const EventCode EventCode::KeyNumpadEnter    = "KeyNumpadEnter";
	const EventCode EventCode::KeyNumpadEqual    = "KeyNumpadEqual";
	const EventCode EventCode::KeyNumLock        = "KeyNumLock";

	// Mouse buttons
	const EventCode EventCode::MouseLeft    = "MouseLeft";
	const EventCode EventCode::MouseRight   = "MouseRight";
	const EventCode EventCode::MouseMiddle  = "MouseMiddle";
	const EventCode EventCode::MouseBack    = "MouseBack";
	const EventCode EventCode::MouseForward = "MouseForward";

	// Mouse axes
	const EventCode EventCode::MouseX      = "MouseX";
	const EventCode EventCode::MouseY      = "MouseY";
	const EventCode EventCode::MouseDeltaX = "MouseDeltaX";
	const EventCode EventCode::MouseDeltaY = "MouseDeltaY";
	const EventCode EventCode::MouseWheelX = "MouseWheelX";
	const EventCode EventCode::MouseWheelY = "MouseWheelY";
	const EventCode EventCode::Text        = "Text";

	// Gamepad buttons
	const EventCode EventCode::GamepadFaceBottom = "GamepadFaceBottom";
	const EventCode EventCode::GamepadFaceRight  = "GamepadFaceRight";
	const EventCode EventCode::GamepadFaceLeft   = "GamepadFaceLeft";
	const EventCode EventCode::GamepadFaceTop    = "GamepadFaceTop";

	const EventCode EventCode::GamepadLeftShoulder  = "GamepadLeftShoulder";
	const EventCode EventCode::GamepadRightShoulder = "GamepadRightShoulder";

	const EventCode EventCode::GamepadLeftStickButton  = "GamepadLeftStickButton";
	const EventCode EventCode::GamepadRightStickButton = "GamepadRightStickButton";

	const EventCode EventCode::GamepadDPadUp    = "GamepadDPadUp";
	const EventCode EventCode::GamepadDPadDown  = "GamepadDPadDown";
	const EventCode EventCode::GamepadDPadLeft  = "GamepadDPadLeft";
	const EventCode EventCode::GamepadDPadRight = "GamepadDPadRight";

	const EventCode EventCode::GamepadStart = "GamepadStart";
	const EventCode EventCode::GamepadBack  = "GamepadBack";
	const EventCode EventCode::GamepadGuide = "GamepadGuide";

	// Gamepad axes
	const EventCode EventCode::GamepadLeftX        = "GamepadLeftX";
	const EventCode EventCode::GamepadLeftY        = "GamepadLeftY";
	const EventCode EventCode::GamepadRightX       = "GamepadRightX";
	const EventCode EventCode::GamepadRightY       = "GamepadRightY";
	const EventCode EventCode::GamepadLeftTrigger  = "GamepadLeftTrigger";
	const EventCode EventCode::GamepadRightTrigger = "GamepadRightTrigger";

}// namespace Trinex
