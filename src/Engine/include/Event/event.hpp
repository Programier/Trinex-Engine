#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class EventType : byte
    {
        // Application events
        Quit,
        AppTerminating,
        AppLowMemory,
        AppWillEnterBackground,
        AppDidEnterBackground,
        AppWillEnterForeground,
        AppDidEnterForeground,
        LocaleChanged,

        // Display events
        Display,

        // Window events
        Window,

        // Keyboard events
        KeyDown,
        KeyUp,
        TextEditing,
        TextInput,
        KeyMapChanged,
        TextEditingExt,

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

        // Gesture events
        DollarGesture,
        DollarRecord,
        MultiGesture,

        // Clipboard events
        ClipboardUpdate,

        // Drag and drop events
        DropFile,
        DropText,
        DropBegin,
        DropComplete,

        // Audio hotplug events
        AudioDeviceAdded,
        AudioDeviceRemoved,

        // Sensor events
        SensorUpdate
    };


    template<typename T, typename = void>
    struct has_type_field : std::false_type {
    };

    template<typename T>
    struct has_type_field<T, std::void_t<decltype(std::declval<T>().type)>> : std::true_type {
    };

    class ENGINE_EXPORT Event
    {
    public:
        using ID = size_t;

    private:
        Any _M_any;
        ID _M_ID;

        EventType _M_type;

        void add_to_id(int_t value, int_t offset = 0);

    public:
        Event(EventType type);

        template<typename Type>
        Event(EventType type, Type&& any) : Event(type)
        {
            _M_any = std::forward<Type>(any);

            if constexpr (std::is_enum_v<Type>)
            {
                add_to_id(static_cast<int_t>(any));
            }
            else if constexpr (has_type_field<Type>::value)
            {
                add_to_id(static_cast<int_t>(any.type));
            }
        }

        EventType type() const;
        bool operator==(const Event& e);
        bool operator!=(const Event& e);
        ID id() const;
        ID base_id() const;
        ID child_id() const;

        template<typename Type>
        Type get() const
        {
            return std::any_cast<Type>(_M_any);
        }
    };
}// namespace Engine
