#pragma once
#include <Graphics/basic_framebuffer.hpp>
#include <Window/window_interface.hpp>

namespace Engine
{
    class ENGINE_EXPORT Window : public BasicFrameBuffer
    {
    private:
        WindowInterface* _M_interface = nullptr;
        Size2D _M_cached_size;

    public:
        using Super = BasicFrameBuffer;

        void close();
        bool is_open();
        Size1D width();
        Window& width(const Size1D& width);
        Size1D height();
        Window& height(const Size1D& height);
        Size2D size();
        Window& size(const Size2D& size);
        String title();
        Window& title(const String& title);
        Point2D position();
        Window& position(const Point2D& position);
        bool resizable();
        Window& resizable(bool value);
        Window& focus();
        bool focused();
        Window& show();
        Window& hide();
        bool is_visible();
        bool is_iconify();
        Window& iconify();
        bool is_restored();
        Window& restore();
        Window& opacity(float value);
        float opacity();
        Window& size_limits(const SizeLimits2D& limits);
        SizeLimits2D size_limits();
        Window& attribute(const WindowAttribute& attrib, bool value);
        bool attribute(const WindowAttribute& attrib);
        Window& cursor_mode(const CursorMode& mode);
        CursorMode cursor_mode();
        bool support_orientation(WindowOrientation orientation);
        Window& start_text_input();
        Window& stop_text_input();
        Window& pool_events();
        Window& wait_for_events();
        Window& swap_buffers();
        Window& update_cached_size();
        const Size2D& cached_size() const;
        void* interface() const;
        Window& icon(const Image& image);
        Window& cursor(const Image& image, IntVector2D hotspot = {0, 0});
        Window& update_monitor_info(MonitorInfo& info);
        int_t create_message_box(const MessageBoxCreateInfo& info);
        Window& create_notify(const NotifyCreateInfo& info);

        void initialize_imgui();
        void terminate_imgui();
        void new_imgui_frame();
        ~Window();

    private:
        Window(WindowInterface* interface);

        friend class Object;
    };
}// namespace Engine
