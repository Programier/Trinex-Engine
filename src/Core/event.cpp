#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/event.hpp>
#include <Core/reflection/enum.hpp>
#include <ScriptEngine/registrar.hpp>
#include <cstring>

namespace Engine
{
	Event::Event()
	{
		std::memset(static_cast<void*>(this), 0, sizeof(Event));
	}

	template<typename T>
	ScriptClassRegistrar::ValueInfo info_of(bool all_ints = false, bool all_floats = false)
	{
		ScriptClassRegistrar::ValueInfo info = {};
		info.pod                             = true;
		info.is_class                        = true;
		info.align8                          = alignof(T) == 8;
		info.all_floats                      = all_floats;
		info.all_ints                        = all_ints;
		return info;
	}

	static void register_event_type()
	{
		ScriptEnumRegistrar r("Engine::EventType");
		using enum EventType;

		r.set("Quit", Quit);
		r.set("AppTerminating", AppTerminating);
		r.set("AppLowMemory", AppLowMemory);
		r.set("AppPause", AppPause);
		r.set("AppResume", AppResume);
		r.set("DisplayAdded", DisplayAdded);
		r.set("DisplayRemoved", DisplayRemoved);
		r.set("DisplayOrientationChanged", DisplayOrientationChanged);
		r.set("WindowShown", WindowShown);
		r.set("WindowHidden", WindowHidden);
		r.set("WindowMoved", WindowMoved);
		r.set("WindowResized", WindowResized);
		r.set("WindowMinimized", WindowMinimized);
		r.set("WindowMaximized", WindowMaximized);
		r.set("WindowRestored", WindowRestored);
		r.set("WindowFocusGained", WindowFocusGained);
		r.set("WindowFocusLost", WindowFocusLost);
		r.set("WindowClose", WindowClose);
		r.set("TextInput", TextInput);
		r.set("KeyDown", KeyDown);
		r.set("KeyUp", KeyUp);
		r.set("MouseMotion", MouseMotion);
		r.set("MouseButtonUp", MouseButtonUp);
		r.set("MouseButtonDown", MouseButtonDown);
		r.set("MouseWheel", MouseWheel);
		r.set("ControllerAxisMotion", ControllerAxisMotion);
		r.set("ControllerButtonUp", ControllerButtonUp);
		r.set("ControllerButtonDown", ControllerButtonDown);
		r.set("ControllerDeviceAdded", ControllerDeviceAdded);
		r.set("ControllerDeviceRemoved", ControllerDeviceRemoved);
		r.set("ControllerDeviceRemapped", ControllerDeviceRemapped);
		r.set("ControllerTouchPadDown", ControllerTouchPadDown);
		r.set("ControllerTouchPadMotion", ControllerTouchPadMotion);
		r.set("ControllerTouchPadUp", ControllerTouchPadUp);
		r.set("ControllerSensorUpdate", ControllerSensorUpdate);
		r.set("FingerDown", FingerDown);
		r.set("FingerUp", FingerUp);
		r.set("FingerMotion", FingerMotion);
		r.set("DropFile", DropFile);
		r.set("DropText", DropText);
		r.set("DropBegin", DropBegin);
		r.set("DropComplete", DropComplete);
	}

	static ReflectionInitializeController enum_initializer(register_event_type, "Engine::EventType");

	static void on_init()
	{
		auto r = ScriptClassRegistrar::value_class("Engine::Event", sizeof(Event), info_of<Event>());

		ScriptClassRegistrar::value_class("Engine::Event::Display", sizeof(Event::Display), info_of<Event::Display>(true))
		        .property("Orientation orientation", &Event::Display::orientation);

		ScriptClassRegistrar::value_class("Engine::Event::Window", sizeof(Event::Window), info_of<Event::Window>(false, true))
		        .property("float x", &Event::Window::x)
		        .property("float y", &Event::Window::y)
		        .property("float width", &Event::Window::width)
		        .property("float height", &Event::Window::height);

		ScriptClassRegistrar::value_class("Engine::Event::Keyboard", sizeof(Event::Keyboard), info_of<Event::Keyboard>(true))
		        .property("Engine::Keyboard::Key key", &Event::Keyboard::key);

		ScriptClassRegistrar::value_class("Engine::Event::Mouse::MouseWheelEvent", sizeof(Event::Mouse::MouseWheelEvent),
		                                  info_of<Event::Mouse::MouseWheelEvent>(false, true))
		        .property("float x", &Event::Mouse::MouseWheelEvent::x)
		        .property("float y", &Event::Mouse::MouseWheelEvent::y);

		ScriptClassRegistrar::value_class("Engine::Event::Mouse::MouseMotionEvent", sizeof(Event::Mouse::MouseMotionEvent),
		                                  info_of<Event::Mouse::MouseMotionEvent>(false, true))
		        .property("float x", &Event::Mouse::MouseMotionEvent::x)
		        .property("float y", &Event::Mouse::MouseMotionEvent::y)
		        .property("float xrel", &Event::Mouse::MouseMotionEvent::xrel)
		        .property("float yrel", &Event::Mouse::MouseMotionEvent::yrel);

		ScriptClassRegistrar::value_class("Engine::Event::Mouse::MouseButtonEvent", sizeof(Event::Mouse::MouseButtonEvent),
		                                  info_of<Event::Mouse::MouseButtonEvent>(false, false))
		        .property("Engine::Mouse::Button button", &Event::Mouse::MouseButtonEvent::button)
		        .property("float x", &Event::Mouse::MouseButtonEvent::x)
		        .property("float y", &Event::Mouse::MouseButtonEvent::y);

		ScriptClassRegistrar::value_class("Engine::Event::Mouse", sizeof(Event::Mouse), info_of<Event::Mouse>(false, false))
		        .property("Engine::Event::Mouse::MouseWheelEvent wheel", &Event::Mouse::wheel)
		        .property("Engine::Event::Mouse::MouseMotionEvent motion", &Event::Mouse::motion)
		        .property("Engine::Event::Mouse::MouseMotionEvent button", &Event::Mouse::button);

		ScriptClassRegistrar::value_class("Engine::Event::Gamepad::AxisMotionEvent", sizeof(Event::Gamepad::AxisMotionEvent),
		                                  info_of<Event::Gamepad::AxisMotionEvent>(false, false))
		        .property("Engine::GameController::Axis axis", &Event::Gamepad::AxisMotionEvent::axis)
		        .property("float value", &Event::Gamepad::AxisMotionEvent::value);

		ScriptClassRegistrar::value_class("Engine::Event::Gamepad", sizeof(Event::Gamepad), info_of<Event::Gamepad>(false, false))
		        .property("uint64 id", &Event::Gamepad::id)
		        .property("Engine::Event::Gamepad::AxisMotionEvent axis_motion", &Event::Gamepad::axis_motion);

		r.property("Engine::Event::Display display", &Event::display);
		r.property("Engine::Event::Window window", &Event::window);
		r.property("Engine::Event::Keyboard keyboard", &Event::keyboard);
		r.property("Engine::Event::Mouse mouse", &Event::mouse);
		r.property("Engine::Event::Gamepad gamepad", &Event::gamepad);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Event",
	                                                  {"Engine::EventType", "Engine::Orientation", "Engine::Keyboard",
	                                                   "Engine::Mouse::Button", "Engine::GameController"});
}// namespace Engine
