#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Image/image.hpp>
#include <SDL_gamecontroller.h>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <window_system.hpp>

namespace Engine
{
    static const Map<WindowAttribute, SDL_WindowFlags> window_attributes = {
            {WindowAttribute::WinResizable, SDL_WINDOW_RESIZABLE},
            {WindowAttribute::WinFullScreen, SDL_WINDOW_FULLSCREEN},
            {WindowAttribute::WinFullScreenDesktop, SDL_WINDOW_FULLSCREEN_DESKTOP},
            {WindowAttribute::WinShown, SDL_WINDOW_SHOWN},
            {WindowAttribute::WinHidden, SDL_WINDOW_HIDDEN},
            {WindowAttribute::WinBorderLess, SDL_WINDOW_BORDERLESS},
            {WindowAttribute::WinMouseFocus, SDL_WINDOW_MOUSE_FOCUS},
            {WindowAttribute::WinInputFocus, SDL_WINDOW_INPUT_FOCUS},
            {WindowAttribute::WinInputGrabbed, SDL_WINDOW_INPUT_GRABBED},
            {WindowAttribute::WinMinimized, SDL_WINDOW_MINIMIZED},
            {WindowAttribute::WinMaximized, SDL_WINDOW_MAXIMIZED},
            {WindowAttribute::WinMouseCapture, SDL_WINDOW_MOUSE_CAPTURE},
            {WindowAttribute::WinAllowHighDPI, SDL_WINDOW_ALLOW_HIGHDPI},
            {WindowAttribute::WinMouseGrabbed, SDL_WINDOW_MOUSE_GRABBED},
            {WindowAttribute::WinKeyboardGrabbed, SDL_WINDOW_KEYBOARD_GRABBED}};

    static Map<Sint32, SDL_GameController*> game_controllers;


#define has_flag(flag) static_cast<bool>(SDL_GetWindowFlags(window) & flag)
    static uint32_t to_sdl_attrib(const Vector<WindowAttribute>& attrib)
    {
        uint32_t value = 0;
        for (auto ell : attrib)
        {
            try
            {
                value |= window_attributes.at(ell);
            }
            catch (...)
            {}
        }
        return value;
    }

    SDL_WindowFlags sdl_api(const String& api_name)
    {
        if (api_name == "OpenGL")
        {
            return SDL_WINDOW_OPENGL;
        }
        else if (api_name == "Vulkan")
        {
            return SDL_WINDOW_VULKAN;
        }

        throw std::runtime_error("Undefined API");
    }

    static void window_initialize_error(const String& msg)
    {
        throw std::runtime_error("Failed to create Window: " + msg);
    }

    void WindowSDL::init(const WindowConfig& info)
    {
        if (window)
            return;


        uint32_t attrib = to_sdl_attrib(info.attributes);
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

        api = sdl_api(info.api_name);

        window = SDL_CreateWindow(info.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  static_cast<int>(info.size.x), static_cast<int>(info.size.y),
                                  api | SDL_WINDOW_SHOWN | attrib);

        if (window == nullptr)
            window_initialize_error(SDL_GetError());
        else
        {
            if (api == SDL_WINDOW_OPENGL)
            {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#ifdef _WIN32
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            }
            else if (api == SDL_WINDOW_VULKAN)
            {}
        }
    }

    void WindowSDL::close()
    {
        if (window)
        {
            destroy_icon();
            destroy_cursor();
            for (auto& pair : game_controllers)
            {
                info_log("WindowSDL", "Force close controller with id %d", pair.first);
                SDL_GameControllerClose(pair.second);
            }

            game_controllers.clear();

            if (gl_context)
            {
                SDL_GL_DeleteContext(gl_context);
                gl_context = nullptr;
            }

            SDL_DestroyWindow(window);
            window = 0;

            SDL_Quit();
        }
    }

    bool WindowSDL::is_open()
    {
        return static_cast<bool>(window);
    }

    Size1D WindowSDL::width()
    {
        return size().x;
    }

    WindowInterface& WindowSDL::width(const Size1D& w)
    {
        size({w, height()});
        return *this;
    }

    Size1D WindowSDL::height()
    {
        return size().y;
    }

    WindowInterface& WindowSDL::height(const Size1D& h)
    {
        size({width(), h});
        return *this;
    }

    Size2D WindowSDL::size()
    {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        return {w, h};
    }

    WindowInterface& WindowSDL::size(const Size2D& size)
    {
        SDL_SetWindowSize(window, size.x, size.y);
        return *this;
    }

    String WindowSDL::title()
    {
        return SDL_GetWindowTitle(window);
    }

    WindowInterface& WindowSDL::title(const String& title)
    {
        SDL_SetWindowTitle(window, title.c_str());
        return *this;
    }

    Point2D WindowSDL::position()
    {
        int x, y;
        SDL_GetWindowPosition(window, &x, &y);
        return {x, y};
    }

    WindowInterface& WindowSDL::position(const Point2D& position)
    {
        SDL_SetWindowPosition(window, position.x, position.y);
        return *this;
    }

    bool WindowSDL::rezisable()
    {
        return has_flag(SDL_WINDOW_RESIZABLE);
    }

    WindowInterface& WindowSDL::rezisable(bool value)
    {
        SDL_SetWindowResizable(window, static_cast<SDL_bool>(value));
        return *this;
    }

    WindowInterface& WindowSDL::focus()
    {
        SDL_SetWindowInputFocus(window);
        return *this;
    }

    bool WindowSDL::focused()
    {
        return has_flag(SDL_WINDOW_INPUT_FOCUS);
    }

    WindowInterface& WindowSDL::show()
    {
        SDL_ShowWindow(window);
        return *this;
    }

    WindowInterface& WindowSDL::hide()
    {
        SDL_HideWindow(window);
        return *this;
    }

    bool WindowSDL::is_visible()
    {
        return has_flag(SDL_WINDOW_SHOWN);
    }

    bool WindowSDL::is_iconify()
    {
        return has_flag(SDL_WINDOW_MINIMIZED);
    }

    WindowInterface& WindowSDL::iconify()
    {
        SDL_MinimizeWindow(window);
        return *this;
    }

    bool WindowSDL::is_restored()
    {
        return !is_iconify();
    }

    WindowInterface& WindowSDL::restore()
    {
        SDL_RestoreWindow(window);
        return *this;
    }

    WindowInterface& WindowSDL::opacity(float value)
    {
        SDL_SetWindowOpacity(window, value);
        return *this;
    }

    float WindowSDL::opacity()
    {
        float o;
        SDL_GetWindowOpacity(window, &o);
        return o;
    }

    WindowInterface& WindowSDL::size_limits(const SizeLimits2D& limits)
    {
        SDL_SetWindowMaximumSize(window, static_cast<int>(limits.max.x), static_cast<int>(limits.max.y));
        SDL_SetWindowMinimumSize(window, static_cast<int>(limits.min.x), static_cast<int>(limits.min.y));
        return *this;
    }

    SizeLimits2D WindowSDL::size_limits()
    {
        SizeLimits2D limits;
        int w, h;

        SDL_GetWindowMaximumSize(window, &w, &h);
        limits.max = {w, h};

        SDL_GetWindowMinimumSize(window, &w, &h);
        limits.min = {w, h};

        return limits;
    }

    void WindowSDL::destroy_icon()
    {
        if (_M_icon)
        {
            SDL_FreeSurface(_M_icon);

            _M_icon = nullptr;
        }
    }

    void WindowSDL::destroy_cursor()
    {
        if (_M_cursor)
        {
            SDL_FreeCursor(_M_cursor);
            _M_cursor = nullptr;
        }

        if (_M_cursor_icon)
        {
            SDL_FreeSurface(_M_cursor_icon);
            _M_cursor_icon = nullptr;
        }
    }

    SDL_Surface* WindowSDL::create_surface(const Buffer& buffer, int_t width, int_t height, int_t channels)
    {

        if (buffer.empty())
            return nullptr;
        void* data = (void*) buffer.data();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        int_t r_mask = 0x000000FF;
        int_t g_mask = 0x0000FF00;
        int_t b_mask = 0x00FF0000;
        int_t a_mask = (channels == 4) ? 0xFF000000 : 0;
#else
        int_t s = (channels == 4) ? 0 : 8;
        int_t r_mask = 0xFF000000 >> s;
        int_t g_mask = 0x00FF0000 >> s;
        int_t b_mask = 0x0000FF00 >> s;
        int_t a_mask = 0x000000FF >> s;
#endif
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data, width, height, channels * 8, width * channels, r_mask,
                                                        g_mask, b_mask, a_mask);
        if (surface == nullptr)
        {
            error_log("WindowSDL", "Failed to create surface from image: %s", SDL_GetError());
        }
        return surface;
    }

    WindowInterface& WindowSDL::window_icon(const Image& image)
    {
        int_t channels = image.channels();
        if (channels == 3 || channels == 4)
        {
            destroy_icon();

            _M_icon_buffer = image.vector();
            _M_icon        = create_surface(_M_icon_buffer, image.width(), image.height(), channels);

            SDL_SetWindowIcon(window, _M_icon);
        }
        else
        {
            error_log("WindowSDL", "Window icon format must be RGB or RGBA!");
        }

        return *this;
    }

    WindowInterface& WindowSDL::cursor(const Image& image, IntVector2D hotspot)
    {
        int_t channels = image.channels();
        if (channels == 3 || channels == 4)
        {
            destroy_cursor();

            _M_cursor_icon_buffer = image.vector();
            _M_cursor_icon        = create_surface(_M_cursor_icon_buffer, image.width(), image.height(), channels);

            if (_M_cursor_icon)
            {
                _M_cursor = SDL_CreateColorCursor(_M_cursor_icon, hotspot.x, hotspot.y);
            }

            SDL_SetCursor(_M_cursor);
        }
        else
        {
            error_log("WindowSDL", "Window icon format must be RGB or RGBA!");
        }

        return *this;
    }

    WindowInterface& WindowSDL::attribute(const WindowAttribute& attrib, bool value)
    {
        try
        {

            static struct {
                bool _M_fullscreen = false;
                Uint32 _M_flag     = 0;
            } fullscreen_mode;

            fullscreen_mode._M_fullscreen = false;
            fullscreen_mode._M_flag       = 0;


            switch (attrib)
            {
                case WindowAttribute::WinResizable:
                    SDL_SetWindowResizable(window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttribute::WinFullScreen:
                    fullscreen_mode._M_flag       = value ? SDL_WINDOW_FULLSCREEN : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttribute::WinFullScreenDesktop:
                    fullscreen_mode._M_flag       = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttribute::WinShown:
                {
                    value ? show() : hide();
                    break;
                }
                case WindowAttribute::WinHidden:
                {
                    value ? hide() : show();
                    break;
                }
                case WindowAttribute::WinBorderLess:
                    SDL_SetWindowBordered(window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttribute::WinInputFocus:
                {
                    if (value)
                        SDL_SetWindowInputFocus(window);
                    break;
                }

                case WindowAttribute::WinMinimized:
                {
                    if (value)
                        SDL_MinimizeWindow(window);
                    break;
                }

                case WindowAttribute::WinMaximized:
                {
                    if (value)
                        SDL_MaximizeWindow(window);
                    break;
                }
                case WindowAttribute::WinMouseGrabbed:
                {
                    SDL_SetWindowMouseGrab(window, static_cast<SDL_bool>(value));
                    break;
                }

                case WindowAttribute::WinKeyboardGrabbed:
                {
                    SDL_SetWindowKeyboardGrab(window, static_cast<SDL_bool>(value));
                    break;
                }

                default:
                    break;
            }


            if (fullscreen_mode._M_fullscreen)
                SDL_SetWindowFullscreen(window, fullscreen_mode._M_flag);
        }
        catch (const std::exception& e)
        {}

        return *this;
    }

    bool WindowSDL::attribute(const WindowAttribute& attrib)
    {
        auto f = window_attributes.at(attrib);
        return has_flag(f);
    }

    WindowInterface& WindowSDL::cursor_mode(const CursorMode& mode)
    {
        if (c_mode != mode)
        {
            SDL_ShowCursor((mode == CursorMode::Hidden ? SDL_DISABLE : SDL_ENABLE));
            c_mode = mode;
        }
        return *this;
    }

    CursorMode WindowSDL::cursor_mode()
    {
        return c_mode;
    }

    WindowInterface& WindowSDL::support_orientation(const Vector<WindowOrientation>& orientations)
    {
        static Map<WindowOrientation, const char*> _M_orientation_map = {
                {WindowOrientation::WinOrientationLandscape, "LandscapeRight"},
                {WindowOrientation::WinOrientationLandscapeFlipped, "LandscapeLeft"},
                {WindowOrientation::WinOrientationPortrait, "Portrait"},
                {WindowOrientation::WinOrientationPortraitFlipped, "PortraitUpsideDown"}};


        String result;
        for (auto ell : orientations)
        {
            if (!result.empty())
                result += " ";
            result += _M_orientation_map.at(ell);
        }

        SDL_SetHint(SDL_HINT_ORIENTATIONS, result.c_str());
        return *this;
    }

    WindowInterface& WindowSDL::start_text_input()
    {
        SDL_StartTextInput();
        return *this;
    }

    WindowInterface& WindowSDL::stop_text_input()
    {
        SDL_StopTextInput();
        return *this;
    }

    WindowInterface& WindowSDL::pool_events()
    {
        while (SDL_PollEvent(&event))
        {
            process_event();
        }

        return *this;
    }

    WindowInterface& WindowSDL::wait_for_events()
    {
        SDL_WaitEvent(&event);
        process_event();
        return *this;
    }


    template<typename Type>
    using ValueMap = const Map<Uint8, Type>;

    static ValueMap<WindowEvent::Type> window_event_types = {
            {SDL_WINDOWEVENT_NONE, WindowEvent::Type::None},
            {SDL_WINDOWEVENT_SHOWN, WindowEvent::Type::Shown},
            {SDL_WINDOWEVENT_HIDDEN, WindowEvent::Type::Hidden},
            {SDL_WINDOWEVENT_EXPOSED, WindowEvent::Type::Exposed},
            {SDL_WINDOWEVENT_MOVED, WindowEvent::Type::Moved},
            {SDL_WINDOWEVENT_RESIZED, WindowEvent::Type::Resized},
            {SDL_WINDOWEVENT_SIZE_CHANGED, WindowEvent::Type::SizeChanged},
            {SDL_WINDOWEVENT_MINIMIZED, WindowEvent::Type::Minimized},
            {SDL_WINDOWEVENT_MAXIMIZED, WindowEvent::Type::Maximized},
            {SDL_WINDOWEVENT_RESTORED, WindowEvent::Type::Restored},
            {SDL_WINDOWEVENT_ENTER, WindowEvent::Type::Enter},
            {SDL_WINDOWEVENT_LEAVE, WindowEvent::Type::Leave},
            {SDL_WINDOWEVENT_FOCUS_GAINED, WindowEvent::Type::FocusGained},
            {SDL_WINDOWEVENT_FOCUS_LOST, WindowEvent::Type::FocusLost},
            {SDL_WINDOWEVENT_CLOSE, WindowEvent::Type::Close},
            {SDL_WINDOWEVENT_TAKE_FOCUS, WindowEvent::Type::TakeFocus},
            {SDL_WINDOWEVENT_HIT_TEST, WindowEvent::Type::HitTest},
            {SDL_WINDOWEVENT_ICCPROF_CHANGED, WindowEvent::Type::IccProfChanged},
            {SDL_WINDOWEVENT_DISPLAY_CHANGED, WindowEvent::Type::DisplayChanged},
    };


    static ValueMap<Keyboard::Key> keys = {
            {SDL_SCANCODE_UNKNOWN, Keyboard::Key::Unknown},
            {SDL_SCANCODE_SPACE, Keyboard::Key::Space},
            {SDL_SCANCODE_APOSTROPHE, Keyboard::Key::Apostrophe},
            {SDL_SCANCODE_COMMA, Keyboard::Key::Comma},
            {SDL_SCANCODE_MINUS, Keyboard::Key::Minus},
            {SDL_SCANCODE_PERIOD, Keyboard::Key::Period},
            {SDL_SCANCODE_SLASH, Keyboard::Key::Slash},
            {SDL_SCANCODE_0, Keyboard::Key::Num0},
            {SDL_SCANCODE_1, Keyboard::Key::Num1},
            {SDL_SCANCODE_2, Keyboard::Key::Num2},
            {SDL_SCANCODE_3, Keyboard::Key::Num3},
            {SDL_SCANCODE_4, Keyboard::Key::Num4},
            {SDL_SCANCODE_5, Keyboard::Key::Num5},
            {SDL_SCANCODE_6, Keyboard::Key::Num6},
            {SDL_SCANCODE_7, Keyboard::Key::Num7},
            {SDL_SCANCODE_8, Keyboard::Key::Num8},
            {SDL_SCANCODE_9, Keyboard::Key::Num9},
            {SDL_SCANCODE_SEMICOLON, Keyboard::Key::Semicolon},
            {SDL_SCANCODE_EQUALS, Keyboard::Key::Equal},
            {SDL_SCANCODE_A, Keyboard::Key::A},
            {SDL_SCANCODE_B, Keyboard::Key::B},
            {SDL_SCANCODE_C, Keyboard::Key::C},
            {SDL_SCANCODE_D, Keyboard::Key::D},
            {SDL_SCANCODE_E, Keyboard::Key::E},
            {SDL_SCANCODE_F, Keyboard::Key::F},
            {SDL_SCANCODE_G, Keyboard::Key::G},
            {SDL_SCANCODE_H, Keyboard::Key::H},
            {SDL_SCANCODE_I, Keyboard::Key::I},
            {SDL_SCANCODE_J, Keyboard::Key::J},
            {SDL_SCANCODE_K, Keyboard::Key::K},
            {SDL_SCANCODE_L, Keyboard::Key::L},
            {SDL_SCANCODE_M, Keyboard::Key::M},
            {SDL_SCANCODE_N, Keyboard::Key::N},
            {SDL_SCANCODE_O, Keyboard::Key::O},
            {SDL_SCANCODE_P, Keyboard::Key::P},
            {SDL_SCANCODE_Q, Keyboard::Key::Q},
            {SDL_SCANCODE_R, Keyboard::Key::R},
            {SDL_SCANCODE_S, Keyboard::Key::S},
            {SDL_SCANCODE_T, Keyboard::Key::T},
            {SDL_SCANCODE_U, Keyboard::Key::U},
            {SDL_SCANCODE_V, Keyboard::Key::V},
            {SDL_SCANCODE_W, Keyboard::Key::W},
            {SDL_SCANCODE_X, Keyboard::Key::X},
            {SDL_SCANCODE_Y, Keyboard::Key::Y},
            {SDL_SCANCODE_Z, Keyboard::Key::Z},
            {SDL_SCANCODE_LEFTBRACKET, Keyboard::Key::LeftBracket},
            {SDL_SCANCODE_BACKSLASH, Keyboard::Key::Backslash},
            {SDL_SCANCODE_RIGHTBRACKET, Keyboard::Key::RightBracket},
            {SDL_SCANCODE_GRAVE, Keyboard::Key::GraveAccent},
            {SDL_SCANCODE_WWW, Keyboard::Key::Www},
            {SDL_SCANCODE_ESCAPE, Keyboard::Key::Escape},
            {SDL_SCANCODE_RETURN, Keyboard::Key::Enter},
            {SDL_SCANCODE_TAB, Keyboard::Key::Tab},
            {SDL_SCANCODE_BACKSPACE, Keyboard::Key::Backspace},
            {SDL_SCANCODE_INSERT, Keyboard::Key::Insert},
            {SDL_SCANCODE_DELETE, Keyboard::Key::Delete},
            {SDL_SCANCODE_RIGHT, Keyboard::Key::Right},
            {SDL_SCANCODE_LEFT, Keyboard::Key::Left},
            {SDL_SCANCODE_DOWN, Keyboard::Key::Down},
            {SDL_SCANCODE_UP, Keyboard::Key::Up},
            {SDL_SCANCODE_PAGEUP, Keyboard::Key::PageUp},
            {SDL_SCANCODE_PAGEDOWN, Keyboard::Key::PageDown},
            {SDL_SCANCODE_HOME, Keyboard::Key::Home},
            {SDL_SCANCODE_END, Keyboard::Key::End},
            {SDL_SCANCODE_CAPSLOCK, Keyboard::Key::CapsLock},
            {SDL_SCANCODE_SCROLLLOCK, Keyboard::Key::ScrollLock},
            {SDL_SCANCODE_NUMLOCKCLEAR, Keyboard::Key::NumLock},
            {SDL_SCANCODE_PRINTSCREEN, Keyboard::Key::PrintScreen},
            {SDL_SCANCODE_PAUSE, Keyboard::Key::Pause},
            {SDL_SCANCODE_F1, Keyboard::Key::F1},
            {SDL_SCANCODE_F2, Keyboard::Key::F2},
            {SDL_SCANCODE_F3, Keyboard::Key::F3},
            {SDL_SCANCODE_F4, Keyboard::Key::F4},
            {SDL_SCANCODE_F5, Keyboard::Key::F5},
            {SDL_SCANCODE_F6, Keyboard::Key::F6},
            {SDL_SCANCODE_F7, Keyboard::Key::F7},
            {SDL_SCANCODE_F8, Keyboard::Key::F8},
            {SDL_SCANCODE_F9, Keyboard::Key::F9},
            {SDL_SCANCODE_F10, Keyboard::Key::F10},
            {SDL_SCANCODE_F11, Keyboard::Key::F11},
            {SDL_SCANCODE_F12, Keyboard::Key::F12},
            {SDL_SCANCODE_F13, Keyboard::Key::F13},
            {SDL_SCANCODE_F14, Keyboard::Key::F14},
            {SDL_SCANCODE_F15, Keyboard::Key::F15},
            {SDL_SCANCODE_F16, Keyboard::Key::F16},
            {SDL_SCANCODE_F17, Keyboard::Key::F17},
            {SDL_SCANCODE_F18, Keyboard::Key::F18},
            {SDL_SCANCODE_F19, Keyboard::Key::F19},
            {SDL_SCANCODE_F20, Keyboard::Key::F20},
            {SDL_SCANCODE_F21, Keyboard::Key::F21},
            {SDL_SCANCODE_F22, Keyboard::Key::F22},
            {SDL_SCANCODE_F23, Keyboard::Key::F23},
            {SDL_SCANCODE_F24, Keyboard::Key::F24},
            {SDL_SCANCODE_KP_0, Keyboard::Key::Kp0},
            {SDL_SCANCODE_KP_1, Keyboard::Key::Kp1},
            {SDL_SCANCODE_KP_2, Keyboard::Key::Kp2},
            {SDL_SCANCODE_KP_3, Keyboard::Key::Kp3},
            {SDL_SCANCODE_KP_4, Keyboard::Key::Kp4},
            {SDL_SCANCODE_KP_5, Keyboard::Key::Kp5},
            {SDL_SCANCODE_KP_6, Keyboard::Key::Kp6},
            {SDL_SCANCODE_KP_7, Keyboard::Key::Kp7},
            {SDL_SCANCODE_KP_8, Keyboard::Key::Kp8},
            {SDL_SCANCODE_KP_9, Keyboard::Key::Kp9},
            {SDL_SCANCODE_KP_DECIMAL, Keyboard::Key::KpDecimal},
            {SDL_SCANCODE_KP_DIVIDE, Keyboard::Key::KpDivide},
            {SDL_SCANCODE_KP_MULTIPLY, Keyboard::Key::KpMultiply},
            {SDL_SCANCODE_KP_MINUS, Keyboard::Key::KpSubtract},
            {SDL_SCANCODE_KP_PLUS, Keyboard::Key::KpAdd},
            {SDL_SCANCODE_KP_ENTER, Keyboard::Key::KpEnter},
            {SDL_SCANCODE_KP_EQUALS, Keyboard::Key::KpEqual},
            {SDL_SCANCODE_LSHIFT, Keyboard::Key::LeftShift},
            {SDL_SCANCODE_LCTRL, Keyboard::Key::LeftControl},
            {SDL_SCANCODE_LALT, Keyboard::Key::LeftAlt},
            {SDL_SCANCODE_LGUI, Keyboard::Key::LeftSuper},
            {SDL_SCANCODE_RSHIFT, Keyboard::Key::RightShift},
            {SDL_SCANCODE_RCTRL, Keyboard::Key::RightControl},
            {SDL_SCANCODE_RALT, Keyboard::Key::RightAlt},
            {SDL_SCANCODE_RGUI, Keyboard::Key::RightSuper},
            {SDL_SCANCODE_MENU, Keyboard::Key::Menu},
    };

    static ValueMap<Mouse::Button> mouse_buttons = {{SDL_BUTTON_LEFT, Mouse::Button::Left},
                                                    {SDL_BUTTON_MIDDLE, Mouse::Button::Middle},
                                                    {SDL_BUTTON_RIGHT, Mouse::Button::Right},
                                                    {SDL_BUTTON_X1, Mouse::Button::X1},
                                                    {SDL_BUTTON_X2, Mouse::Button::X2}};


    static ValueMap<GameController::Axis> axis_type = {
            {SDL_CONTROLLER_AXIS_INVALID, GameController::Axis::None},
            {SDL_CONTROLLER_AXIS_LEFTX, GameController::Axis::LeftX},
            {SDL_CONTROLLER_AXIS_LEFTY, GameController::Axis::LeftY},
            {SDL_CONTROLLER_AXIS_TRIGGERLEFT, GameController::Axis::TriggerLeft},
            {SDL_CONTROLLER_AXIS_RIGHTX, GameController::Axis::RightX},
            {SDL_CONTROLLER_AXIS_RIGHTY, GameController::Axis::RightY},
            {SDL_CONTROLLER_AXIS_TRIGGERLEFT, GameController::Axis::TriggerRight},
    };

    template<typename MapType, typename KeyType, bool& result>
    static decltype(auto) value_at(MapType&& map, KeyType&& key)
    {
        try
        {
            result = true;
            return map.at(std::forward<KeyType>(key));
        }
        catch (...)
        {
            error_log("SDL Window System", "Cannot get value '%d' from map", int(key));
            result = false;
            return decltype(map.at(key))();
        }
    }

#define new_event(a, b) send_event(Event(EventType::a, b))

    void WindowSDL::process_mouse_button()
    {
        MouseButtonEvent button_event;
        button_event.clicks = event.button.clicks;
        button_event.x      = event.button.x;
        button_event.y      = event.button.y;

        auto it = mouse_buttons.find(event.button.button);
        if (it != mouse_buttons.end())
        {
            button_event.button = it->second;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            new_event(MouseButtonDown, button_event);
        }
        else
        {
            new_event(MouseButtonUp, button_event);
        }
    }


    void WindowSDL::process_event()
    {
        for (auto func : on_event)
        {
            func(&event);
        }


        switch (event.type)
        {
            case SDL_QUIT:
                new_event(Quit, QuitEvent());
                break;

            case SDL_APP_TERMINATING:
                new_event(AppTerminating, AppTerminatingEvent());
                break;

            case SDL_APP_LOWMEMORY:
                new_event(Quit, AppLowMemoryEvent());
                break;

            case SDL_APP_WILLENTERBACKGROUND:
                new_event(AppWillEnterBackground, AppWillEnterBackgroundEvent());
                break;

            case SDL_APP_DIDENTERBACKGROUND:
                new_event(AppDidEnterBackground, AppDidEnterBackgroundEvent());
                break;

            case SDL_APP_WILLENTERFOREGROUND:
                new_event(AppWillEnterForeground, AppWillEnterForegroundEvent());
                break;

            case SDL_APP_DIDENTERFOREGROUND:
                new_event(AppDidEnterForeground, AppDidEnterForegroundEvent());
                break;

            case SDL_LOCALECHANGED:
                new_event(LocaleChanged, LocaleChangedEvent());
                break;

            case SDL_DISPLAYEVENT:
            {
                break;
            }

            case SDL_WINDOWEVENT:
            {
                auto it = window_event_types.find(event.window.event);
                if (it != window_event_types.end())
                {
                    WindowEvent engine_event;
                    engine_event.type = it->second;

                    engine_event.x = event.window.data1;
                    engine_event.y = event.window.data2;

                    new_event(Window, engine_event);
                }
                break;
            }

            case SDL_KEYDOWN:
            {
                KeyEvent key_event;
                auto it = keys.find(event.key.keysym.scancode);
                if (it != keys.end())
                {
                    key_event.key    = it->second;
                    key_event.repeat = event.key.repeat;
                    new_event(KeyDown, key_event);
                }
                else
                {
                    error_log("SDL Window System", "Cannot find scancode '%d'", event.key.keysym.scancode);
                }
                break;
            }

            case SDL_KEYUP:
            {
                KeyEvent key_event;
                auto it = keys.find(event.key.keysym.scancode);
                if (it != keys.end())
                {
                    key_event.key    = it->second;
                    key_event.repeat = event.key.repeat;
                    new_event(KeyUp, key_event);
                }
                else
                {
                    error_log("SDL Window System", "Cannot find scancode '%d'", event.key.keysym.scancode);
                }
                break;
            }


            case SDL_MOUSEMOTION:
            {
                MouseMotionEvent mouse_motion;
                mouse_motion.x    = event.motion.x;
                mouse_motion.y    = height() - event.motion.y;
                mouse_motion.xrel = event.motion.xrel;
                mouse_motion.yrel = -event.motion.yrel;

                new_event(MouseMotion, mouse_motion);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                process_mouse_button();
                break;
            }

            case SDL_MOUSEWHEEL:
            {
                MouseWheelEvent wheel_event;
                wheel_event.x         = event.wheel.preciseX;
                wheel_event.y         = event.wheel.preciseY;
                wheel_event.direction = event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? Mouse::Normal : Mouse::Flipped;
                new_event(MouseWheel, wheel_event);
                break;
            }

            case SDL_CONTROLLERDEVICEADDED:
            {
                game_controllers[event.cdevice.which] = SDL_GameControllerOpen(event.cdevice.which);
                ControllerDeviceAddedEvent c_event;
                c_event.id = static_cast<Identifier>(event.cdevice.which) + 1;
                new_event(ControllerDeviceAdded, c_event);
                break;
            }

            case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_GameControllerClose(game_controllers[event.cdevice.which]);
                game_controllers.erase(event.cdevice.which);
                ControllerDeviceAddedEvent c_event;
                c_event.id = static_cast<Identifier>(event.cdevice.which) + 1;
                new_event(ControllerDeviceRemoved, c_event);
                break;
            }

            case SDL_CONTROLLERAXISMOTION:
            {
                ControllerAxisMotionEvent motion_event;
                motion_event.id = static_cast<Identifier>(event.caxis.which) + 1;
                try
                {
                    motion_event.axis = axis_type.at(event.caxis.axis);
                }
                catch (...)
                {
                    motion_event.axis = GameController::Axis::None;
                }

                motion_event.value = event.caxis.value;
                new_event(ControllerAxisMotion, motion_event);
                break;
            }

#define kek(x)                                                                                                         \
    case x:                                                                                                            \
    {                                                                                                                  \
        info_log("SDL", #x);                                                                                           \
        break;                                                                                                         \
    }

                kek(SDL_CONTROLLERBUTTONDOWN);
                kek(SDL_CONTROLLERBUTTONUP);
                kek(SDL_CONTROLLERDEVICEREMAPPED);
                kek(SDL_CONTROLLERTOUCHPADDOWN);
                kek(SDL_CONTROLLERTOUCHPADMOTION);
                kek(SDL_CONTROLLERTOUCHPADUP);
                kek(SDL_CONTROLLERSENSORUPDATE);


                //                    SDL_TEXTEDITING,
                //                    SDL_TEXTINPUT,
                //                    SDL_KEYMAPCHANGED,
                //                    SDL_TEXTEDITING_EXT,


                //                    SDL_FINGERDOWN,
                //                    SDL_FINGERUP,
                //                    SDL_FINGERMOTION,

                //                    SDL_DOLLARGESTURE,
                //                    SDL_DOLLARRECORD,
                //                    SDL_MULTIGESTURE,

                //                    SDL_CLIPBOARDUPDATE,

                //                    SDL_DROPFILE,
                //                    SDL_DROPTEXT,
                //                    SDL_DROPBEGIN,
                //                    SDL_DROPCOMPLETE,

                //                    SDL_AUDIODEVICEADDED,
                //                    SDL_AUDIODEVICEREMOVED,

                //                    SDL_SENSORUPDATE,

                //                    SDL_RENDER_TARGETS_RESET,
                //                    SDL_RENDER_DEVICE_RESET,

                //                    SDL_POLLSENTINEL,

                //                    SDL_USEREVENT,

                //                    SDL_LASTEVENT
        }
    }


    void* WindowSDL::create_surface(const char* any_text, ...)
    {
        if (api == SDL_WINDOW_VULKAN)
        {
            va_list args;
            va_start(args, any_text);
            VkInstance instance = va_arg(args, VkInstance);
            va_end(args);

            SDL_Vulkan_CreateSurface(window, instance, &vulkan_surface);
            return &vulkan_surface;
        }
        else if (api == SDL_WINDOW_OPENGL)
        {
            gl_context = SDL_GL_CreateContext(window);
            SDL_GL_MakeCurrent(window, gl_context);
            if (!gl_context)
            {
                throw std::runtime_error(SDL_GetError());
            }
            return gl_context;
        }

        return nullptr;
    }

    WindowInterface& WindowSDL::swap_buffers()
    {
        if (api == SDL_WINDOW_OPENGL)
        {
            SDL_GL_SwapWindow(window);
        }
        return *this;
    }

    Vector<const char*> WindowSDL::required_extensions()
    {
        if (api == SDL_WINDOW_VULKAN)
        {
            uint32_t count = 0;

            SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
            Vector<const char*> extensions(count);
            SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
            return extensions;
        }

        return {};
    }

    WindowInterface& WindowSDL::add_event_callback(Identifier system_id, const EventCallback& callback)
    {
        event_callbacks[system_id].push_back(callback);
        return *this;
    }

    WindowInterface& WindowSDL::remove_all_callbacks(Identifier system_id)
    {
        event_callbacks.erase(system_id);
        return *this;
    }

    bool WindowSDL::mouse_relative_mode() const
    {
        return static_cast<bool>(SDL_GetRelativeMouseMode());
    }

    WindowInterface& WindowSDL::mouse_relative_mode(bool flag)
    {
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(flag));
        return *this;
    }


    void WindowSDL::send_event(const Event& event)
    {
        for (auto& system_callbacks : event_callbacks)
        {
            for (auto& callback : system_callbacks.second)
            {
                callback(event);
            }
        }
    }

    static void process_imgui_event(SDL_Event* event)
    {
        ImGui_ImplSDL2_ProcessEvent(event);
    }

    void WindowSDL::initialize_imgui_opengl()
    {
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    }

    void WindowSDL::initialize_imgui_vulkan()
    {
        ImGui_ImplSDL2_InitForVulkan(window);
    }

    WindowInterface& WindowSDL::initialize_imgui()
    {
        IMGUI_CHECKVERSION();
        imgui_context = ImGui::CreateContext();


        switch (api)
        {
            case SDL_WINDOW_OPENGL:
                initialize_imgui_opengl();
                break;

            case SDL_WINDOW_VULKAN:
                initialize_imgui_vulkan();
                break;

            default:
                break;
        }

        on_event.insert(process_imgui_event);
        return *this;
    }

    WindowInterface& WindowSDL::terminate_imgui()
    {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(imgui_context);
        on_event.erase(process_imgui_event);
        return *this;
    }

    WindowInterface& WindowSDL::new_imgui_frame()
    {
        ImGui_ImplSDL2_NewFrame();
        return *this;
    }

    WindowSDL::~WindowSDL()
    {
        close();
    }

    WindowInterface& WindowSDL::update_monitor_info(MonitorInfo& info)
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(0, &mode);
        info.height       = mode.h;
        info.width        = mode.w;
        info.refresh_rate = mode.refresh_rate;
        SDL_GetDisplayDPI(0, &info.dpi.ddpi, &info.dpi.hdpi, &info.dpi.vdpi);
        return *this;
    }

}// namespace Engine


#ifndef ENABLE_ENGINE_EXPORTS
#define ENABLE_ENGINE_EXPORTS
#endif


extern "C" ENGINE_EXPORT Engine::WindowInterface* load_window_system()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    return new Engine::WindowSDL();
}
