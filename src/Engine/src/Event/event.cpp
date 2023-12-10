#include <Core/engine_loading_controllers.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>


namespace Engine
{
    static const String event_names[] = {
            // Application events
            "Quit",
            "AppTerminating",
            "AppLowMemory",
            "AppWillEnterBackground",
            "AppDidEnterBackground",
            "AppWillEnterForeground",
            "AppDidEnterForeground",
            "LocaleChanged",

            // Display events
            "Display",

            // Window events
            "Window",

            // Keyboard events
            "KeyDown",
            "KeyUp",
            "TextEditing",
            "TextInput",
            "KeyMapChanged",
            "TextEditingExt",

            // Mouse events
            "MouseMotion",
            "MouseButtonUp",
            "MouseButtonDown",
            "MouseWheel",

            // Game controller events
            "ControllerAxisMotion",
            "ControllerButtonUp",
            "ControllerButtonDown",
            "ControllerDeviceAdded",
            "ControllerDeviceRemoved",
            "ControllerDeviceRemapped",
            "ControllerTouchPadDown",
            "ControllerTouchPadMotion",
            "ControllerTouchPadUp",
            "ControllerSensorUpdate",

            // Touch events
            "FingerDown",
            "FingerUp",
            "FingerMotion",

            // Gesture events
            "DollarGesture",
            "DollarRecord",
            "MultiGesture",

            // Clipboard events
            "ClipboardUpdate",

            // Drag and drop events
            "DropFile",
            "DropText",
            "DropBegin",
            "DropComplete",

            // Audio hotplug events
            "AudioDeviceAdded",
            "AudioDeviceRemoved",

            // Sensor events
            "SensorUpdate",
    };

    static inline constexpr size_t event_count = sizeof(event_names) / sizeof(event_names[0]);


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

    const String& Event::name() const
    {
        if (static_cast<size_t>(_M_type) < event_count)
        {
            return event_names[static_cast<size_t>(_M_type)];
        }

        static String undefined = "undefined";
        return undefined;
    }

    const Any& Event::any() const
    {
        return _M_any;
    }
}// namespace Engine
