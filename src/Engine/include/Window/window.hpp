#pragma once
#include <Core/buffer_types.hpp>
#include <Core/callback.hpp>
#include <Core/color.hpp>
#include <Core/export.hpp>
#include <Core/keyboard.hpp>
#include <Event/event.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Window/cursor.hpp>
#include <functional>
#include <string>



namespace Engine
{

    using AspectRation = glm::vec<2, std::int32_t, glm::defaultp>;

    enum WindowAttrib : uint16_t
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

    enum class CursorMode
    {
        Normal,
        Hidden
    };

    enum WindowOrientation : uint_t
    {
        WinOrientationLandscape        = 1,
        WinOrientationLandscapeFlipped = 2,
        WinOrientationPortrait         = 4,
        WinOrientationPortraitFlipped  = 8

    };


    class ENGINE_EXPORT Window : public BasicFrameBuffer
    {
    public:
        static Event event;
        // Window struct methods
        static const Window& init(float width, float height, const String& title = STR(""), uint16_t attrib = 0);
        static const Window& init(const Size2D& size, const String& title = STR(""), uint16_t attrib = 0);
        static const Window& close();
        static bool is_open();
        static const Window& swap_buffers();

        static Size1D width();
        static const Window& width(const Size1D& width);
        static Size1D height();
        static const Window& height(const Size1D& height);
        static const Size2D& size();
        static const Window& size(const Size2D& size);


        static int_t swap_interval();
        static const Window& swap_interval(int_t value);

        static bool vsync();
        static const Window& vsync(const bool& value);

        static const String& title();
        static const Window& title(const String& title);

        static const Point2D& position();
        static const Window& position(const Point2D& position);

        static const Vector<String>& dropped_paths();
        static const Window& clear_dropped_paths();

        static bool rezisable();
        static const Window& rezisable(bool value);

        static const Window& focus();
        static bool focused();

        static const Window& show();
        static const Window& hide();
        static bool is_visible();

        static bool is_iconify();
        static const Window& iconify();
        static bool is_restored();
        static const Window& restore();

        static const Window& opacity(float value);
        static float opacity();

        static bool center_cursor();

        static const Window& size_limits(const SizeLimits2D& limits);
        static const SizeLimits2D& size_limits();

        static const Window& cursor(const Cursor& cursor);
        static const Cursor& cursor();

        static const Window& icon(const Image& image);
        static const Window& icon(const String& image);
        static const Image& icon();

        static const Window& aspect_ration(const AspectRation& ration);
        static const AspectRation& aspect_ration();

        static const Window& attribute(const WindowAttrib& attrib, bool value);
        static bool attribute(const WindowAttrib& attrib);

        static const Window& cursor_mode(const CursorMode& mode);
        static const CursorMode& cursor_mode();

        static const Window& bind();
        static const Window& update_view_port();
        static const Window& update_scissor();

        static const Window& X11_compositing(bool value);

        static void* SDL();
        static void* SDL_OpenGL_context();

        static const Window& set_orientation(uint_t orientation);
        static const Window& start_text_input();
        static const Window& stop_text_input();
        static const Window& update_viewport_on_resize(bool value);
        static bool update_viewport_on_resize();
        static const Window& update_scissor_on_resize(bool value);
        static bool update_scissor_on_resize();
        static CallBacks<void> on_resize;
        static std::size_t frame_number();
        static BasicFrameBuffer* framebuffer();

        // Constructors

        Window();
        Window(float width, float height, const String& title = STR(""), uint16_t attrib = 0);
        Window(const Size2D& size, const String& title = STR(""), uint16_t attrib = 0);
        Window(const Window& window);
        Window& operator=(const Window& window);
        ~Window();


        friend class Application;
        friend class EngineInstance;

    private:
        static const Window& destroy_window();
    };


}// namespace Engine
