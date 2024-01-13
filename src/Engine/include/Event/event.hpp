#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

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
        WindowNone,
        WindowShown,
        WindowHidden,
        WindowExposed,
        WindowMoved,
        WindowResized,
        WindowSizeChanged,
        WindowMinimized,
        WindowMaximized,
        WindowRestored,
        WindowEnter,
        WindowLeave,
        WindowFocusGained,
        WindowFocusLost,
        WindowClose,
        WindowTakeFocus,
        WindowHitTest,
        WindowIccProfChanged,
        WindowDisplayChanged,

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


    class ENGINE_EXPORT Event
    {
    private:
        Any _M_any;
        Identifier _M_window_id;
        EventType _M_type;


    public:
        Event();
        copy_constructors_hpp(Event);
        Event(Identifier window_id, EventType type);
        Event(Identifier window_id, EventType type, const Any& any);

        template<typename T>
        Event(Identifier window_id, EventType type, T&& value) : Event(window_id, type)
        {
            _M_any = std::forward<T>(value);
        }

        EventType type() const;
        Identifier window_id() const;
        const Any& any() const;

        template<typename Type>
        Type get()
        {
            return _M_any.cast<Type>();
        }

        template<typename Type>
        const Type get() const
        {
            return _M_any.cast<const Type>();
        }
    };
}// namespace Engine
