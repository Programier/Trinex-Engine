#pragma once
#include <Core/engine_types.hpp>
#include <Core/game_controller.hpp>
#include <Core/keyboard.hpp>
#include <Core/mouse.hpp>
#include <Core/structures.hpp>

namespace Engine
{
    using EmptyEvent = EmptyStruct;

    using QuitEvent           = EmptyEvent;
    using AppTerminatingEvent = EmptyEvent;
    using AppLowMemoryEvent   = EmptyEvent;
    using AppPauseEvent       = EmptyEvent;
    using AppResumeEvent      = EmptyEvent;

    struct DisplayAddedEvent {
    };

    struct DisplayRemovedEvent {
    };

    struct ENGINE_EXPORT DisplayOrientationChangedEvent {
        WindowOrientation orientation;
    };

    using WindowShownEvent  = EmptyStruct;
    using WindowHiddenEvent = EmptyEvent;

    struct ENGINE_EXPORT WindowEvent {
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

    using WindowMovedEvent       = WindowEvent;
    using WindowResizedEvent     = WindowEvent;
    using WindowMinimizedEvent   = EmptyEvent;
    using WindowMaximizedEvent   = EmptyEvent;
    using WindowRestoredEvent    = EmptyEvent;
    using WindowFocusGainedEvent = EmptyEvent;
    using WindowFocusLostEvent   = EmptyEvent;
    using WindowCloseEvent       = EmptyEvent;


    struct ENGINE_EXPORT KeyEvent {
        Keyboard::Key key;
    };

    using KeyDownEvent = KeyEvent;
    using KeyUpEvent   = KeyEvent;

    struct ENGINE_EXPORT MouseMotionEvent {
        int32_t x;
        int32_t y;
        int32_t xrel;
        int32_t yrel;
    };

    struct ENGINE_EXPORT MouseButtonEvent {
        Mouse::Button button;
        int_t x;
        int_t y;
    };


    using MouseButtonUpEvent   = MouseButtonEvent;
    using MouseButtonDownEvent = MouseButtonEvent;

    struct ENGINE_EXPORT MouseWheelEvent {
        float x;
        float y;
    };


    // Game controller events
    struct ENGINE_EXPORT EventWithControllerId {
        Identifier id;
    };

    struct ENGINE_EXPORT ControllerAxisMotionEvent : EventWithControllerId {
        GameController::Axis axis;
        short_t value;
    };

    // struct ENGINE_EXPORT ControllerButtonEvent : EventWithControllerId {
    //     GameController::LeftX axis;
    //     short_t value;
    // };

    //         ControllerButtonUp,
    //         ControllerButtonDown,

    //         using ControllerDeviceAddedEvent   = EventWithControllerId;
    //         using ControllerDeviceRemovedEvent = EventWithControllerId;
    //         ControllerDeviceRemapped,
    //         ControllerTouchPadDown,
    //         ControllerTouchPadMotion,
    //         ControllerTouchPadUp,
    //         ControllerSensorUpdate,

    //         // Touch events
    //         FingerDown,
    //         FingerUp,
    //         FingerMotion,

    //         // Drag and drop events
    //         DropFile,
    //         DropText,
    //         DropBegin,
    //         DropComplete,

}// namespace Engine
