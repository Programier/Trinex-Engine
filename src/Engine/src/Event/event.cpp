#include <Core/engine_loading_controllers.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <ScriptEngine/registrar.hpp>>


namespace Engine
{
    Event::Event(Identifier window_id, EventType type) : _M_window_id(window_id), _M_type(type)
    {}

    EventType Event::type() const
    {
        return _M_type;
    }

    Identifier Event::window_id() const
    {
        return _M_window_id;
    }

    const Any& Event::any() const
    {
        return _M_any;
    }


    static InitializeController on_init([]() {
        ScriptEnumRegistrar registrar("Engine::EventType");
        registrar.set("Quit", EventType::Quit);
        registrar.set("AppTerminating", EventType::AppTerminating);
        registrar.set("AppLowMemory", EventType::AppLowMemory);
        registrar.set("AppWillEnterBackground", EventType::AppWillEnterBackground);
        registrar.set("AppDidEnterBackground", EventType::AppDidEnterBackground);
        registrar.set("AppWillEnterForeground", EventType::AppWillEnterForeground);
        registrar.set("AppDidEnterForeground", EventType::AppDidEnterForeground);
        registrar.set("LocaleChanged", EventType::LocaleChanged);
        registrar.set("Display", EventType::Display);
        registrar.set("WindowNone", EventType::WindowNone);
        registrar.set("WindowShown", EventType::WindowShown);
        registrar.set("WindowHidden", EventType::WindowHidden);
        registrar.set("WindowExposed", EventType::WindowExposed);
        registrar.set("WindowMoved", EventType::WindowMoved);
        registrar.set("WindowResized", EventType::WindowResized);
        registrar.set("WindowSizeChanged", EventType::WindowSizeChanged);
        registrar.set("WindowMinimized", EventType::WindowMinimized);
        registrar.set("WindowMaximized", EventType::WindowMaximized);
        registrar.set("WindowRestored", EventType::WindowRestored);
        registrar.set("WindowEnter", EventType::WindowEnter);
        registrar.set("WindowLeave", EventType::WindowLeave);
        registrar.set("WindowFocusGained", EventType::WindowFocusGained);
        registrar.set("WindowFocusLost", EventType::WindowFocusLost);
        registrar.set("WindowClose", EventType::WindowClose);
        registrar.set("WindowTakeFocus", EventType::WindowTakeFocus);
        registrar.set("WindowHitTest", EventType::WindowHitTest);
        registrar.set("WindowIccProfChanged", EventType::WindowIccProfChanged);
        registrar.set("WindowDisplayChanged", EventType::WindowDisplayChanged);
        registrar.set("KeyDown", EventType::KeyDown);
        registrar.set("KeyUp", EventType::KeyUp);
        registrar.set("TextEditing", EventType::TextEditing);
        registrar.set("TextInput", EventType::TextInput);
        registrar.set("KeyMapChanged", EventType::KeyMapChanged);
        registrar.set("TextEditingExt", EventType::TextEditingExt);
        registrar.set("MouseMotion", EventType::MouseMotion);
        registrar.set("MouseButtonUp", EventType::MouseButtonUp);
        registrar.set("MouseButtonDown", EventType::MouseButtonDown);
        registrar.set("MouseWheel", EventType::MouseWheel);
        registrar.set("ControllerAxisMotion", EventType::ControllerAxisMotion);
        registrar.set("ControllerButtonUp", EventType::ControllerButtonUp);
        registrar.set("ControllerButtonDown", EventType::ControllerButtonDown);
        registrar.set("ControllerDeviceAdded", EventType::ControllerDeviceAdded);
        registrar.set("ControllerDeviceRemoved", EventType::ControllerDeviceRemoved);
        registrar.set("ControllerDeviceRemapped", EventType::ControllerDeviceRemapped);
        registrar.set("ControllerTouchPadDown", EventType::ControllerTouchPadDown);
        registrar.set("ControllerTouchPadMotion", EventType::ControllerTouchPadMotion);
        registrar.set("ControllerTouchPadUp", EventType::ControllerTouchPadUp);
        registrar.set("ControllerSensorUpdate", EventType::ControllerSensorUpdate);
        registrar.set("FingerDown", EventType::FingerDown);
        registrar.set("FingerUp", EventType::FingerUp);
        registrar.set("FingerMotion", EventType::FingerMotion);
        registrar.set("DollarGesture", EventType::DollarGesture);
        registrar.set("DollarRecord", EventType::DollarRecord);
        registrar.set("MultiGesture", EventType::MultiGesture);
        registrar.set("ClipboardUpdate", EventType::ClipboardUpdate);
        registrar.set("DropFile", EventType::DropFile);
        registrar.set("DropText", EventType::DropText);
        registrar.set("DropBegin", EventType::DropBegin);
        registrar.set("DropComplete", EventType::DropComplete);
        registrar.set("AudioDeviceAdded", EventType::AudioDeviceAdded);
        registrar.set("AudioDeviceRemoved", EventType::AudioDeviceRemoved);
        registrar.set("SensorUpdat", EventType::SensorUpdate);
    }, "Bind Event");
}// namespace Engine
