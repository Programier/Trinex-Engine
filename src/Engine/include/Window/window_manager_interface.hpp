#pragma once
#include <Core/structures.hpp>


namespace Engine
{
    struct WindowConfig;
    struct WindowInterface;
    struct MonitorInfo;
    class Event;
    class SplashConfig;

    using EventCallback = Function<void(const Event&)>;

    class WindowManagerInterface
    {
    public:
        virtual WindowInterface* create_window(const WindowConfig* config)                                      = 0;
        virtual WindowManagerInterface& destroy_window(WindowInterface* interface)                              = 0;
        virtual WindowManagerInterface& create_notify(const NotifyCreateInfo& info)                             = 0;
        virtual String error() const                                                                            = 0;
        virtual bool has_error() const                                                                          = 0;
        virtual bool mouse_relative_mode() const                                                                = 0;
        virtual WindowManagerInterface& mouse_relative_mode(bool flag)                                          = 0;
        virtual WindowManagerInterface& update_monitor_info(MonitorInfo& info)                                  = 0;
        virtual WindowManagerInterface& add_event_callback(Identifier system_id, const EventCallback& callback) = 0;
        virtual WindowManagerInterface& remove_all_callbacks(Identifier system_id)                              = 0;
        virtual WindowManagerInterface& start_text_input()                                                      = 0;
        virtual WindowManagerInterface& stop_text_input()                                                       = 0;
        virtual WindowManagerInterface& pool_events()                                                           = 0;
        virtual WindowManagerInterface& wait_for_events()                                                       = 0;

        // Splash Screen
        virtual bool show_splash_screen(const class Image& image, Size2D splash_size, const SplashConfig& config) = 0;
        virtual WindowManagerInterface& update_splash_screen()                                                    = 0;
        virtual WindowManagerInterface& update_splash_screen_text(SplashTextType type, const StringView& text)    = 0;
        virtual WindowManagerInterface& hide_splash_screen()                                                      = 0;

        virtual ~WindowManagerInterface() = default;
    };
}// namespace Engine
