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

    void Event::add_to_id(int_t value, int_t offset)
    {
        _M_ID |= static_cast<ID>(value + 1) << offset;
    }

    Event::Event(EventType type) : _M_any({}), _M_ID(0), _M_type(type)
    {
        add_to_id(static_cast<int_t>(type), sizeof(ID) * 4);
    }

    EventType Event::type() const
    {
        return _M_type;
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

    bool Event::operator==(const Event& e)
    {
        return _M_ID == e._M_ID;
    }

    bool Event::operator!=(const Event& e)
    {
        return _M_ID != e._M_ID;
    }

    Event::ID Event::id() const
    {
        return _M_ID;
    }

    Event::ID Event::base_id() const
    {
        constexpr ID mask = (~static_cast<ID>(0)) << (sizeof(ID) * 4);
        return id() & mask;
    }

    Event::ID Event::child_id() const
    {
        constexpr ID mask = (~static_cast<ID>(0)) >> (sizeof(ID) * 4);
        return id() & mask;
    }

    const Any& Event::any() const
    {
        return _M_any;
    }


    template<typename... Args>
    static Event* static_create_event(EventType type, const Args&... args)
    {
        info_log("Event", "%s", __PRETTY_FUNCTION__);
        return new Event(type, args...);
    }
}// namespace Engine
