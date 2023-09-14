#pragma once
#include <Core/engine_types.hpp>
#include <Core/keyboard.hpp>
#include <Core/mouse.hpp>
#include <Core/game_controller.hpp>

namespace Engine
{
    using EmptyEvent = EmptyStruct;

    using QuitEvent                   = EmptyEvent;
    using AppTerminatingEvent         = EmptyEvent;
    using AppLowMemoryEvent           = EmptyEvent;
    using AppWillEnterBackgroundEvent = EmptyEvent;
    using AppDidEnterBackgroundEvent  = EmptyEvent;
    using AppWillEnterForegroundEvent = EmptyEvent;
    using AppDidEnterForegroundEvent  = EmptyEvent;
    using LocaleChangedEvent          = EmptyEvent;

    struct DisplayEvent {
    };


    struct WindowEvent {
        enum Type
        {
            None,
            Shown,
            Hidden,
            Exposed,
            Moved,
            Resized,
            SizeChanged,
            Minimized,
            Maximized,
            Restored,
            Enter,
            Leave,
            FocusGained,
            FocusLost,
            Close,
            TakeFocus,
            HitTest,
            IccProfChanged,
            DisplayChanged
        };

        Type type;

        union
        {
            int_t x;
            int_t width;
        };

        union
        {
            int_t y;
            int_t height;
        };
    };

    struct KeyEvent {
        union
        {
            Keyboard::Key key;
            Keyboard::Key type;
        };

        bool repeat = false;
    };


    struct MouseMotionEvent {
        int32_t x;
        int32_t y;
        int32_t xrel;
        int32_t yrel;
    };

    struct MouseButtonEvent {
        union
        {
            Mouse::Button button;
            Mouse::Button type;
        };

        uint_t clicks;
        int_t x;
        int_t y;
    };

    struct MouseWheelEvent {
        Mouse::Direction direction;
        float x;
        float y;
    };


    struct EventWithControllerId {
        union
        {
            Identifier id;
            Identifier type;
        };
    };

    using ControllerDeviceAddedEvent   = EventWithControllerId;
    using ControllerDeviceRemovedEvent = EventWithControllerId;

    struct ControllerAxisMotionEvent : EventWithControllerId {
        GameController::Axis axis;
        short_t value;
    };

    /*
    // Display events
    DisplayEvent,

            // Window events
            WindowEvent, SysWMEvent,

            // Keyboard events
            KeyDown, KeyUp, TextEditing, TextInput, KeyMapChanged, TextEditingExt,

            // Mouse events
            MouseMotion, MouseButtonUp, MouseButtonDown, MouseWheel,



            // Game controller events
            ControllerAxisMotion, ControllerButtonUp, ControllerButtonDown, ControllerDeviceAdded,
            ControllerDeviceRemoved, ControllerDeviceRemapped, ControllerTouchPadDown, ControllerTouchPadMotion,
            ControllerTouchPadUp, ControllerSensorUpdate,

            // Touch events
            FingerDown, FingerUp, FingerMotion,

            // Gesture events
            DollarGesture, DollarRecord, MultiGesture,

            // Clipboard events
            ClipboardUpdate,

            // Drag and drop events
            DropFile, DropText, DropBegin, DropComplete,

            // Audio hotplug events
            AudioDeviceAdded, AudioDeviceRemoved,

            // Sensor events
            SensorUpdate* /
*/
}// namespace Engine
