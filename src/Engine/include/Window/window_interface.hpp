#pragma once


#include <Core/structures.hpp>
#include <Event/event.hpp>


namespace Engine
{
    class Image;
    using EventCallback = Function<void(const Event&)>;
    struct MonitorInfo;
    struct WindowConfig;

    struct WindowInterface {
        virtual void init(const WindowConfig& info)                                                      = 0;
        virtual void close()                                                                             = 0;
        virtual bool is_open()                                                                           = 0;
        virtual Size1D width()                                                                           = 0;
        virtual WindowInterface& width(const Size1D& width)                                              = 0;
        virtual Size1D height()                                                                          = 0;
        virtual WindowInterface& height(const Size1D& height)                                            = 0;
        virtual Size2D size()                                                                            = 0;
        virtual WindowInterface& size(const Size2D& size)                                                = 0;
        virtual String title()                                                                           = 0;
        virtual WindowInterface& title(const String& title)                                              = 0;
        virtual Point2D position()                                                                       = 0;
        virtual WindowInterface& position(const Point2D& position)                                       = 0;
        virtual bool resizable()                                                                         = 0;
        virtual WindowInterface& resizable(bool value)                                                   = 0;
        virtual WindowInterface& focus()                                                                 = 0;
        virtual bool focused()                                                                           = 0;
        virtual WindowInterface& show()                                                                  = 0;
        virtual WindowInterface& hide()                                                                  = 0;
        virtual bool is_visible()                                                                        = 0;
        virtual bool is_iconify()                                                                        = 0;
        virtual WindowInterface& iconify()                                                               = 0;
        virtual bool is_restored()                                                                       = 0;
        virtual WindowInterface& restore()                                                               = 0;
        virtual WindowInterface& opacity(float value)                                                    = 0;
        virtual float opacity()                                                                          = 0;
        virtual WindowInterface& size_limits(const SizeLimits2D& limits)                                 = 0;
        virtual SizeLimits2D size_limits()                                                               = 0;
        virtual WindowInterface& window_icon(const Image& image)                                         = 0;
        virtual WindowInterface& cursor(const Image& image, IntVector2D hotspot = {0, 0})                = 0;
        virtual WindowInterface& attribute(const WindowAttribute& attrib, bool value)                    = 0;
        virtual bool attribute(const WindowAttribute& attrib)                                            = 0;
        virtual WindowInterface& cursor_mode(const CursorMode& mode)                                     = 0;
        virtual CursorMode cursor_mode()                                                                 = 0;
        virtual bool support_orientation(WindowOrientation orientation)                                  = 0;
        virtual WindowInterface& start_text_input()                                                      = 0;
        virtual WindowInterface& stop_text_input()                                                       = 0;
        virtual WindowInterface& pool_events()                                                           = 0;
        virtual WindowInterface& wait_for_events()                                                       = 0;
        virtual void* create_surface(const char* any_text, ...)                                          = 0;
        virtual WindowInterface& initialize_imgui()                                                      = 0;
        virtual WindowInterface& terminate_imgui()                                                       = 0;
        virtual WindowInterface& new_imgui_frame()                                                       = 0;
        virtual WindowInterface& swap_buffers()                                                          = 0;
        virtual Vector<const char*> required_extensions()                                                = 0;
        virtual WindowInterface& add_event_callback(Identifier system_id, const EventCallback& callback) = 0;
        virtual WindowInterface& remove_all_callbacks(Identifier system_id)                              = 0;
        virtual bool mouse_relative_mode() const                                                         = 0;
        virtual WindowInterface& mouse_relative_mode(bool flag)                                          = 0;
        virtual WindowInterface& update_monitor_info(MonitorInfo& info)                                  = 0;
        virtual WindowInterface& vsync(bool)                                                             = 0;
        virtual bool vsync()                                                                             = 0;
        virtual int_t create_message_box(const MessageBoxCreateInfo& info)                               = 0;
        virtual WindowInterface& create_notify(const NotifyCreateInfo& info)                             = 0;

        virtual ~WindowInterface() = default;
    };
}// namespace Engine
