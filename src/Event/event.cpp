#include <Core/engine_loading_controllers.hpp>
#include <Core/enum.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	Event::Event() = default;
	default_copy_constructors_cpp(Event);
	Event::Event(Identifier window_id, EventType type) : m_window_id(window_id), m_type(type)
	{}

	Event::Event(Identifier window_id, EventType type, const Any& any) : m_any(any), m_window_id(window_id), m_type(type)
	{}

	EventType Event::type() const
	{
		return m_type;
	}

	Identifier Event::window_id() const
	{
		return m_window_id;
	}

	const Any& Event::any() const
	{
		return m_any;
	}

	implement_enum(EventType, Engine, {"Quit", EventType::Quit}, {"AppTerminating", EventType::AppTerminating},
				   {"AppLowMemory", EventType::AppLowMemory}, {"AppPause", EventType::AppPause},
				   {"AppResume", EventType::AppResume}, {"DisplayAdded", EventType::DisplayAdded},
				   {"DisplayRemoved", EventType::DisplayRemoved},
				   {"DisplayOrientationChanged", EventType::DisplayOrientationChanged}, {"WindowShown", EventType::WindowShown},
				   {"WindowHidden", EventType::WindowHidden}, {"WindowMoved", EventType::WindowMoved},
				   {"WindowResized", EventType::WindowResized}, {"WindowMinimized", EventType::WindowMinimized},
				   {"WindowMaximized", EventType::WindowMaximized}, {"WindowRestored", EventType::WindowRestored},
				   {"WindowFocusGained", EventType::WindowFocusGained}, {"WindowFocusLost", EventType::WindowFocusLost},
				   {"WindowClose", EventType::WindowClose}, {"TextInput", EventType::TextInput}, {"KeyDown", EventType::KeyDown},
				   {"KeyUp", EventType::KeyUp}, {"MouseMotion", EventType::MouseMotion},
				   {"MouseButtonUp", EventType::MouseButtonUp}, {"MouseButtonDown", EventType::MouseButtonDown},
				   {"MouseWheel", EventType::MouseWheel}, {"ControllerAxisMotion", EventType::ControllerAxisMotion},
				   {"ControllerButtonUp", EventType::ControllerButtonUp},
				   {"ControllerButtonDown", EventType::ControllerButtonDown},
				   {"ControllerDeviceAdded", EventType::ControllerDeviceAdded},
				   {"ControllerDeviceRemoved", EventType::ControllerDeviceRemoved},
				   {"ControllerDeviceRemapped", EventType::ControllerDeviceRemapped},
				   {"ControllerTouchPadDown", EventType::ControllerTouchPadDown},
				   {"ControllerTouchPadMotion", EventType::ControllerTouchPadMotion},
				   {"ControllerTouchPadUp", EventType::ControllerTouchPadUp},
				   {"ControllerSensorUpdate", EventType::ControllerSensorUpdate}, {"FingerDown", EventType::FingerDown},
				   {"FingerUp", EventType::FingerUp}, {"FingerMotion", EventType::FingerMotion},
				   {"DropFile", EventType::DropFile}, {"DropText", EventType::DropText}, {"DropBegin", EventType::DropBegin},
				   {"DropComplete", EventType::DropComplete});

	static void on_init()
	{
		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("Engine::Event", sizeof(Event));

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Event>,
						 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(uint64, EventType, const any&)",
						 ScriptClassRegistrar::constructor<Event, Identifier, EventType, const Any&>,
						 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const Event&)",
						 ScriptClassRegistrar::constructor<Event, const Event&>, ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Event>,
						 ScriptCallConv::CDeclObjFirst);
		registrar.method("Event& opAssign(const Event&)", method_of<Event&, const Event&>(&Event::operator=));
		registrar.method("EventType type() const", &Event::type);
		registrar.method("uint64 window_id() const", &Event::window_id);
		registrar.method("const any& any() const", &Event::any);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Event", {"Engine::EventType"});
}// namespace Engine
