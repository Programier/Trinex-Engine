#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/any.hpp>

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
        Event(Identifier window_id, EventType type);

        template<typename Type>
        Event(Identifier window_id, EventType type, Type&& any) : Event(window_id, type)
        {
            _M_any = std::forward<Type>(any);
        }

        EventType type() const;
        Identifier window_id() const;
        const Any& any() const;

        template<typename Type>
        Type get() const
        {
            return _M_any.get<Type>();
        }
    };
}// namespace Engine
