#pragma once
#include "Image/image.hpp"
#include "color.hpp"
#include "cursor.hpp"
#include "keyboard.hpp"
#include <glm/glm.hpp>
#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

#define METHODS(name, type)                                                                                            \
    name();                                                                                                            \
    name(const type&, const type&);                                                                                    \
    name(const name&);                                                                                                 \
    name(const std::initializer_list<type>& list);                                                                     \
    name& operator=(const name&);

namespace Engine
{
    enum CursorStatus
    {
        NORMAL,
        DISABLED,
        HIDDEN
    };

    typedef glm::vec2 WindowSize;
    typedef glm::vec2 Position2D;
    typedef glm::vec2 Offset;

    std::ostream& print(const glm::vec2& vector, std::ostream& stream);
    enum WindowMode
    {
        NONE,
        WIN_FULLSCREEN,
        FULLSCREEN
    };

    std::ostream& operator<<(std::ostream& stream, const WindowMode& mode);

    struct SizeLimits {
        WindowSize min_size;
        WindowSize max_size;
        METHODS(SizeLimits, WindowSize);
    };

    std::ostream& operator<<(std::ostream& stream, const SizeLimits& limits);

    struct AspectRation {
        int numer;
        int denom;
        METHODS(AspectRation, int);
    };

    std::ostream& operator<<(std::ostream& stream, const AspectRation& ar);

    struct WindowParameters {
        // Window parameters
        void* _M_window;
        std::string _M_name;
        bool _M_is_closed;
        std::vector<std::string> _M_dropped_paths;
        SizeLimits _M_limits;
        WindowMode _M_mode = NONE;
        std::vector<int> _M_backup = {};
        Color _M_color;
        AspectRation _M_aspect_ration;

        // Keyboard parameters
        struct {
            KeyStatus* _M_keys;
            int _M_last_key = -1;
            int _M_last_released;

            int _M_last_mouse_key = -1;
            int _M_last_mouse_released;
        } keys;
        unsigned int _M_last_symbol;

        // Mouse parameters
        Position2D _M_mouse_position;
        Offset _M_offset;
        Offset _M_scroll_offset;
        CursorStatus _M_cursor_status = CursorStatus::NORMAL;


        // Icon
        Image _M_icon;

        // Cursor
        Cursor _M_cursor;

        float _M_diff_time = 0;
        WindowParameters(const std::string& name);
        WindowParameters(const WindowParameters& param);
        ~WindowParameters();
    };

    class Window
    {
    private:
        WindowParameters parameters;
        void event_preprocessing();

    public:
        class WindowEvent
        {
            Engine::Window* window;

        public:
            class Keyboard
            {
                Engine::Window* window;

            public:
                Keyboard(Window*);
                const Key just_pressed();
                const std::string& last_symbol();
                const Key last_pressed();
                const Key just_released();
            } keyboard;

            class Mouse
            {
                Engine::Window* window;

            public:
                Mouse(Window* window);
                const Position2D& position();
                Window& position(const Position2D& position);
                const Offset& offset();
                const Offset& scroll_offset();
                Window& cursor_status(const CursorStatus& status);
                CursorStatus& cursor_status();

                const Key just_pressed();
                const Key last_pressed();
                const Key just_released();

            } mouse;

            KeyStatus get_key_status(const Key& key);
            bool pressed(const Key& key);
            WindowEvent(Engine::Window*);
            static void poll_events();
            float diff_time();
        } event;


        Window(int width, int height, std::string name = "", bool rezisable = true);
        Window(const Window& window) = delete;
        bool is_open() const;
        void clear_buffer();
        void swap_buffers();
        void make_current();

        int width();
        Window& width(const int& width);
        int height();
        Window& height(const int& height);
        WindowSize size();
        Window& size(const WindowSize& size);

        void clear_dropped_path();
        const std::vector<std::string>& get_dropped_paths();
        bool rezisable();
        Window& rezisable(bool value);

        const std::string& title();
        Window& title(const std::string& title);

        const WindowMode& mode();
        Window& mode(const WindowMode& mode, const WindowSize& size = WindowSize(-1, -1));

        const Position2D& position();
        Window& position(const Position2D& position);

        void focus();
        bool focused();
        void show();
        void hide();
        bool is_visible();
        Color& background_color();
        Window& background_color(const Color& color);
        bool is_iconify();
        Window& iconify();
        bool is_restored();
        Window& restore();
        Window& opacity(float value);
        float opacity();
        bool center_cursor();
        Window& center_cursor(bool value);
        Window& size_limits(const SizeLimits& limits);
        const SizeLimits& size_limits();
        void close();
        void use_default_cursor();
        Window& aspect_ration(const AspectRation& ration);
        const AspectRation& aspect_ration();
        void disable_aspect_ration();
        Window& icon(const std::string& path);
        const Image& icon();
        Window& icon(const Image& icon);
        Window& cursor(const Cursor& cursor);
        const Cursor& cursor();
        void destroy();
        friend WindowParameters& get_parameters(Window* window);
        ~Window();
    };
}// namespace Engine
