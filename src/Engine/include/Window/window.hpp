#pragma once
#include <Window/cursor.hpp>
#include <Window/keyboard.hpp>
#include <engine.hpp>
#include <functional>
#include <string>
#include <vector>

namespace Engine
{

    struct Window {
        // Window Event system
        struct Event {

            struct Keyboard {
                static const Key just_pressed();
                static unsigned int last_symbol(bool reset = true);
                static const Key last_pressed();
                static const Key just_released();
            } keyboard;

            struct Mouse {
                static const Point2D& position();
                static const Window& position(const Point2D& position);
                static const Offset2D& offset();
                static const Offset2D& scroll_offset();

                static const Key just_pressed();
                static const Key last_pressed();
                static const Key just_released();
            } mouse;

            static const Window& poll_events();
            static const Window& wait_for_events();
            static float diff_time();
            static float time();
            static KeyStatus get_key_status(const Key& key);
            static bool pressed(const Key& key);
        } event;

        struct Callbacks
        {
            static std::function<void(const Size2D&)>& resize_callback();
            static std::function<void(const Point2D&)>& position_callback();
        } callbacks;

        // Window struct methods
        static const Window& init(float width, float height, const std::string& title = "", bool rezisable = true);
        static const Window& init(const Size2D& size, const std::string& title = "", bool rezisable = true);
        static const Window& close();
        static bool is_open();
        static const Window& swap_buffers();

        static Size1D width();
        static const Window& width(const Size1D& width);
        static Size1D height();
        static const Window& height(const Size1D& height);
        static const Size2D& size();
        static const Window& size(const Size2D& size);


        static int swap_interval();
        static const Window& swap_interval(int value);

        static bool vsync();
        static const Window& vsync(const bool& value);

        static const std::string& title();
        static const Window& title(const std::string& title);

        static const Point2D& position();
        static const Window& position(const Point2D& position);

        static const std::vector<std::string>& dropped_paths();
        static const Window& clear_dropped_paths();

        static bool rezisable();
        static const Window& rezisable(bool value);

        static const Window& focus();
        static bool focused();

        static const Window& show();
        static const Window& hide();
        static bool is_visible();

        static Color& background_color();
        static const Window& background_color(const Color& color);

        static const Window& clear_buffer(const BufferType& buffer = COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

        static bool is_iconify();
        static const Window& iconify();
        static bool is_restored();
        static const Window& restore();

        static const Window& opacity(float value);
        static float opacity();

        static bool center_cursor();
        static const Window& center_cursor(bool value);

        static const Window& size_limits(const SizeLimits2D& limits);
        static const SizeLimits2D& size_limits();

        static const Window& cursor(const Cursor& cursor);
        static const Cursor& cursor();

        static const Window& icon(const Image& image);
        static const Window& icon(const std::string& image);
        static const Image& icon();

        static const Window& aspect_ration(const AspectRation& ration);
        static const AspectRation& aspect_ration();

        static const Window& attribute(const WindowAttrib& attrib, bool value);
        static bool attribute(const WindowAttrib& attrib);

        static const WindowMode& mode();
        static const Window& mode(const WindowMode& mode, const Size2D& size = {-1, -1});

        static const Window& cursor_mode(const CursorMode& mode);
        static const CursorMode& cursor_mode();

        static const Window& bind();
        static const Window& update_view_port();


        // Constructors

        Window();
        Window(float width, float height, const std::string& title = "", bool rezisable = true);
        Window(const Size2D& size, const std::string& title = "", bool rezisable = true);
        Window(const Window& window);
        Window& operator=(const Window& window);
        ~Window();
    };


}// namespace Engine
