#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/enum.hpp>
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

	implement_engine_enum(EventType, EventType::Quit, EventType::AppTerminating, EventType::AppLowMemory, EventType::AppPause,
						  EventType::AppResume, EventType::DisplayAdded, EventType::DisplayRemoved,
						  EventType::DisplayOrientationChanged, EventType::WindowShown, EventType::WindowHidden,
						  EventType::WindowMoved, EventType::WindowResized, EventType::WindowMinimized,
						  EventType::WindowMaximized, EventType::WindowRestored, EventType::WindowFocusGained,
						  EventType::WindowFocusLost, EventType::WindowClose, EventType::TextInput, EventType::KeyDown,
						  EventType::KeyUp, EventType::MouseMotion, EventType::MouseButtonUp, EventType::MouseButtonDown,
						  EventType::MouseWheel, EventType::ControllerAxisMotion, EventType::ControllerButtonUp,
						  EventType::ControllerButtonDown, EventType::ControllerDeviceAdded, EventType::ControllerDeviceRemoved,
						  EventType::ControllerDeviceRemapped, EventType::ControllerTouchPadDown,
						  EventType::ControllerTouchPadMotion, EventType::ControllerTouchPadUp, EventType::ControllerSensorUpdate,
						  EventType::FingerDown, EventType::FingerUp, EventType::FingerMotion, EventType::DropFile,
						  EventType::DropText, EventType::DropBegin, EventType::DropComplete);


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
