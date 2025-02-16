#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/event.hpp>
#include <Core/reflection/enum.hpp>
#include <ScriptEngine/registrar.hpp>
#include <cstring>

namespace Engine
{
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
		ScriptClassRegistrar::ValueInfo info = ScriptClassRegistrar::ValueInfo::from<Event>();
		info.pod                             = true;
		ScriptClassRegistrar registrar       = ScriptClassRegistrar::value_class("Engine::Event", sizeof(Event), info);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Event", {"Engine::EventType"});
}// namespace Engine
