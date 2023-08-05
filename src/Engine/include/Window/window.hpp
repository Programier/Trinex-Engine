#pragma once
#include <Core/buffer_types.hpp>
#include <Core/callback.hpp>
#include <Core/color.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/keyboard.hpp>
#include <Event/event.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Window/cursor.hpp>


struct SDL_Window;
struct SDL_WindowEvent;
struct SDL_DropEvent;

namespace Engine
{

    enum WindowAttrib : EnumerateType
    {
        WinNone                   = 0,
        WinResizable              = 1,
        WinFullScreen             = 2,
        WinFullScreenDesktop      = 4,
        WinShown                  = 8,
        WinHidden                 = 16,
        WinBorderLess             = 32,
        WinMouseFocus             = 64,
        WinInputFocus             = 128,
        WinInputGrabbed           = 256,
        WinMinimized              = 512,
        WinMaximized              = 1024,
        WinTransparentFramebuffer = 2048,
        WinMouseCapture           = 4096,
        WinAllowHighDPI           = 8192,
        WinMouseGrabbed           = 16384,
        WinKeyboardGrabbed        = 32768,
    };

    enum class CursorMode : EnumerateType
    {
        Normal,
        Hidden
    };

    enum WindowOrientation : EnumerateType
    {
        WinOrientationLandscape        = 1,
        WinOrientationLandscapeFlipped = 2,
        WinOrientationPortrait         = 4,
        WinOrientationPortraitFlipped  = 8
    };


    class ENGINE_EXPORT Window : public BasicFrameBuffer, public Singletone<Window>
    {
    private:
        static Window* _M_instance;

        Cursor _M_cursor;
        Image _M_icon;
        String _M_title;
        Vector<String> _M_dropped_paths;
        SizeLimits2D _M_limits;

        Size2D _M_size = {-1, -1};
        Size2D _M_position;

        SDL_Window* _M_window               = nullptr;
        const char* _M_X11_compositing      = "0";
        void* _M_api_context                = nullptr;
        struct SDL_Surface* _M_icon_surface = nullptr;

        int_t _M_swap_interval = 1;
        uint_t _M_flags;

        CursorMode _M_cursor_mode = CursorMode::Normal;

        bool _M_is_inited : 1                  = false;
        bool _M_enable_ration : 1              = false;
        bool _M_is_transparent_framebuffer : 1 = false;
        bool _M_change_viewport_on_resize : 1  = true;
        bool _M_update_scissor_on_resize : 1   = true;
        bool _M_api_inited : 1                 = false;

        Window* free_icon_surface();
        void process_window_event(SDL_WindowEvent& event);
        void process_paths_event(SDL_DropEvent& event);

    public:
        using Super = BasicFrameBuffer;

        static Window* window;
        Event event;
        CallBacks<void()> on_resize;

        // Window struct methodsb
        Window* init(float width, float height, const String& title = STR(""), uint16_t attrib = 0);
        Window* init(const Size2D& size, const String& title = STR(""), uint16_t attrib = 0);
        Window* close();
        bool is_open() const;
        const Window* swap_buffers() const;

        Size1D width() const;
        const Window* width(const Size1D& width) const;
        Size1D height() const;
        const Window* height(const Size1D& height) const;
        const Size2D& size() const;
        const Window* size(const Size2D& size) const;


        int_t swap_interval() const;
        Window* swap_interval(int_t value);

        bool vsync() const;
        Window* vsync(bool value);

        const String& title() const;
        Window* title(const String& title);
        const Point2D& position() const;
        const Window* position(const Point2D& position) const;
        const Vector<String>& dropped_paths() const;
        Window* clear_dropped_paths();
        bool rezisable() const;
        const Window* rezisable(bool value) const;
        const Window* focus() const;
        bool focused() const;
        const Window* show() const;
        const Window* hide() const;
        bool is_visible() const;
        bool is_iconify() const;
        const Window* iconify() const;
        bool is_restored() const;
        const Window* restore() const;
        const Window* opacity(float value) const;
        float opacity() const;
        Window* size_limits(const SizeLimits2D& limits);
        const SizeLimits2D& size_limits() const;
        Window* cursor(const Cursor& cursor);
        const Cursor& cursor() const;
        Window* icon(const Image& image);
        Window* icon(const String& image);
        const Image& icon() const;
        Window* attribute(const WindowAttrib& attrib, bool value);
        bool attribute(const WindowAttrib& attrib) const;
        Window* cursor_mode(const CursorMode& mode);
        CursorMode cursor_mode() const;
        Window* update_view_port();
        Window* update_scissor();
        Window* X11_compositing(bool value);
        void* SDL() const;
        void* api_context() const;
        Window* set_orientation(uint_t orientation);
        const Window* start_text_input() const;
        const Window* stop_text_input() const;
        Window* update_viewport_on_resize(bool value);
        bool update_viewport_on_resize() const;
        Window* update_scissor_on_resize(bool value);
        bool update_scissor_on_resize() const;
        Window* initialize_api();
        bool is_api_initialized() const;
        size_t frame_number();

        static void on_class_register(void*);

        // Constructors

    private:
        Window();
        ~Window();

        friend class EngineInstance;
        friend struct UpdateEvents;
        friend class Singletone;
        friend class Object;

        Window* destroy_window();
    };


}// namespace Engine
