#pragma once
#include <Core/structures.hpp>


namespace Engine
{
    struct WindowConfig;
    struct WindowInterface;
    struct MonitorInfo;
    class Event;
    struct SplashConfig;

    using EventCallback = Function<void(const Event&)>;

    class WindowManagerInterface
    {
    public:
        virtual WindowInterface* create_window(const WindowConfig* config)         = 0;
        virtual WindowManagerInterface& destroy_window(WindowInterface* interface) = 0;
        virtual bool mouse_relative_mode() const                                   = 0;
        virtual WindowManagerInterface& mouse_relative_mode(bool flag)             = 0;
        virtual WindowManagerInterface& update_monitor_info(MonitorInfo& info)     = 0;
        virtual WindowManagerInterface& pool_events()                              = 0;
        virtual WindowManagerInterface& wait_for_events()                          = 0;

        virtual ~WindowManagerInterface() = default;
    };
}// namespace Engine
