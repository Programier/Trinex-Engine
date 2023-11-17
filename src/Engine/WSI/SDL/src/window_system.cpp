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
            {WindowAttribute::Resizable, SDL_WINDOW_RESIZABLE},
            {WindowAttribute::FullScreen, SDL_WINDOW_FULLSCREEN},
            {WindowAttribute::FullScreenDesktop, SDL_WINDOW_FULLSCREEN_DESKTOP},
            {WindowAttribute::Shown, SDL_WINDOW_SHOWN},
            {WindowAttribute::Hidden, SDL_WINDOW_HIDDEN},
            {WindowAttribute::BorderLess, SDL_WINDOW_BORDERLESS},
            {WindowAttribute::MouseFocus, SDL_WINDOW_MOUSE_FOCUS},
            {WindowAttribute::InputFocus, SDL_WINDOW_INPUT_FOCUS},
            {WindowAttribute::InputGrabbed, SDL_WINDOW_INPUT_GRABBED},
            {WindowAttribute::Minimized, SDL_WINDOW_MINIMIZED},
            {WindowAttribute::Maximized, SDL_WINDOW_MAXIMIZED},
            {WindowAttribute::MouseCapture, SDL_WINDOW_MOUSE_CAPTURE},
            {WindowAttribute::AllowHighDPI, SDL_WINDOW_ALLOW_HIGHDPI},
            {WindowAttribute::MouseGrabbed, SDL_WINDOW_MOUSE_GRABBED},
            {WindowAttribute::KeyboardGrabbed, SDL_WINDOW_KEYBOARD_GRABBED}};

    static Map<Sint32, SDL_GameController*> game_controllers;


#define has_flag(flag) static_cast<bool>(SDL_GetWindowFlags(_M_window) & flag)
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
        if (_M_window)
            return;
        uint32_t attrib = to_sdl_attrib(info.attributes);
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

        _M_api = sdl_api(info.api_name);

        _M_window = SDL_CreateWindow(info.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     static_cast<int>(info.size.x), static_cast<int>(info.size.y),
                                     _M_api | SDL_WINDOW_SHOWN | attrib);

        if (_M_window == nullptr)
            window_initialize_error(SDL_GetError());
        else
        {
            if (_M_api == SDL_WINDOW_OPENGL)
            {

#if PLATFORM_ANDROID || PLATFORM_LINUX
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
                SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            }
            else if (_M_api == SDL_WINDOW_VULKAN)
            {
            }
        }
    }

    void WindowSDL::close()
    {
        if (_M_window)
        {
            destroy_icon();
            destroy_cursor();
            for (auto& pair : game_controllers)
            {
                info_log("WindowSDL", "Force close controller with id %d", pair.first);
                SDL_GameControllerClose(pair.second);
            }

            game_controllers.clear();

            if (_M_gl_context)
            {
                SDL_GL_DeleteContext(_M_gl_context);
                _M_gl_context = nullptr;
            }

            SDL_DestroyWindow(_M_window);
            _M_window = 0;

            SDL_Quit();
        }
    }

    bool WindowSDL::is_open()
    {
        return static_cast<bool>(_M_window);
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
        SDL_GetWindowSize(_M_window, &w, &h);
        return {w, h};
    }

    WindowInterface& WindowSDL::size(const Size2D& size)
    {
        SDL_SetWindowSize(_M_window, size.x, size.y);
        return *this;
    }

    String WindowSDL::title()
    {
        return SDL_GetWindowTitle(_M_window);
    }

    WindowInterface& WindowSDL::title(const String& title)
    {
        SDL_SetWindowTitle(_M_window, title.c_str());
        return *this;
    }

    Point2D WindowSDL::position()
    {
        int x, y;
        SDL_GetWindowPosition(_M_window, &x, &y);
        return {x, y};
    }

    WindowInterface& WindowSDL::position(const Point2D& position)
    {
        SDL_SetWindowPosition(_M_window, position.x, position.y);
        return *this;
    }

    bool WindowSDL::resizable()
    {
        return has_flag(SDL_WINDOW_RESIZABLE);
    }

    WindowInterface& WindowSDL::resizable(bool value)
    {
        SDL_SetWindowResizable(_M_window, static_cast<SDL_bool>(value));
        return *this;
    }

    WindowInterface& WindowSDL::focus()
    {
        SDL_SetWindowInputFocus(_M_window);
        return *this;
    }

    bool WindowSDL::focused()
    {
        return has_flag(SDL_WINDOW_INPUT_FOCUS);
    }

    WindowInterface& WindowSDL::show()
    {
        SDL_ShowWindow(_M_window);
        return *this;
    }

    WindowInterface& WindowSDL::hide()
    {
        SDL_HideWindow(_M_window);
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
        SDL_MinimizeWindow(_M_window);
        return *this;
    }

    bool WindowSDL::is_restored()
    {
        return !is_iconify();
    }

    WindowInterface& WindowSDL::restore()
    {
        SDL_RestoreWindow(_M_window);
        return *this;
    }

    WindowInterface& WindowSDL::opacity(float value)
    {
        SDL_SetWindowOpacity(_M_window, value);
        return *this;
    }

    float WindowSDL::opacity()
    {
        float o;
        SDL_GetWindowOpacity(_M_window, &o);
        return o;
    }

    WindowInterface& WindowSDL::size_limits(const SizeLimits2D& limits)
    {
        SDL_SetWindowMaximumSize(_M_window, static_cast<int>(limits.max.x), static_cast<int>(limits.max.y));
        SDL_SetWindowMinimumSize(_M_window, static_cast<int>(limits.min.x), static_cast<int>(limits.min.y));
        return *this;
    }

    SizeLimits2D WindowSDL::size_limits()
    {
        SizeLimits2D limits;
        int w, h;

        SDL_GetWindowMaximumSize(_M_window, &w, &h);
        limits.max = {w, h};

        SDL_GetWindowMinimumSize(_M_window, &w, &h);
        limits.min = {w, h};

        return limits;
    }

    WindowSDL& WindowSDL::vsync(bool flag)
    {
        if (_M_api == SDL_WINDOW_OPENGL)
        {
            SDL_GL_SetSwapInterval(static_cast<int>(flag));
        }
        return *this;
    }

    bool WindowSDL::vsync()
    {
        if (_M_api == SDL_WINDOW_OPENGL)
        {
            return SDL_GL_GetSwapInterval() != 0;
        }

        throw EngineException("Cannot use vsync method from window interface, when API is not OpenGL");
    }

    int_t WindowSDL::create_message_box(const MessageBoxCreateInfo& info)
    {
        SDL_MessageBoxData data;
        data.window  = _M_window;
        data.title   = info.title.c_str();
        data.message = info.message.c_str();

        switch (info.type)
        {
            case MessageBoxType::Info:
                data.flags = SDL_MESSAGEBOX_INFORMATION;
                break;

            case MessageBoxType::Warning:
                data.flags = SDL_MESSAGEBOX_WARNING;
                break;

            case MessageBoxType::Error:
                data.flags = SDL_MESSAGEBOX_ERROR;
                break;

            default:
                data.flags = SDL_MESSAGEBOX_INFORMATION;
                break;
        }

        Vector<SDL_MessageBoxButtonData> buttons;
        buttons.reserve(info.buttons.size());

        for (auto& button : info.buttons)
        {
            buttons.emplace_back();
            SDL_MessageBoxButtonData& sdl_button = buttons.back();

            sdl_button.buttonid = button.id;
            sdl_button.flags    = 0;
            sdl_button.text     = button.name.c_str();
        }

        data.numbuttons = static_cast<decltype(data.numbuttons)>(buttons.size());
        data.buttons    = buttons.data();

        SDL_MessageBoxColorScheme sheme;

        sheme.colors[SDL_MESSAGEBOX_COLOR_BACKGROUND] = {static_cast<Uint8>(info.sheme.background.r),
                                                         static_cast<Uint8>(info.sheme.background.r),
                                                         static_cast<Uint8>(info.sheme.background.r)};

        sheme.colors[SDL_MESSAGEBOX_COLOR_TEXT] = {static_cast<Uint8>(info.sheme.text.r),
                                                   static_cast<Uint8>(info.sheme.text.r),
                                                   static_cast<Uint8>(info.sheme.text.r)};

        sheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] = {static_cast<Uint8>(info.sheme.button_border.r),
                                                            static_cast<Uint8>(info.sheme.button_border.r),
                                                            static_cast<Uint8>(info.sheme.button_border.r)};

        sheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] = {static_cast<Uint8>(info.sheme.button_background.r),
                                                                static_cast<Uint8>(info.sheme.button_background.r),
                                                                static_cast<Uint8>(info.sheme.button_background.r)};

        sheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] = {static_cast<Uint8>(info.sheme.button_selected.r),
                                                              static_cast<Uint8>(info.sheme.button_selected.r),
                                                              static_cast<Uint8>(info.sheme.button_selected.r)};

        data.colorScheme = &sheme;

        int button_id = 0;
        if (SDL_ShowMessageBox(&data, &button_id) < 0)
        {
            SDL_Log("SDL_ShowMessageBox Error: %s", SDL_GetError());
            return ~0;
        }

        return button_id;
    }

    void create_system_notify(const NotifyCreateInfo& info);

    WindowSDL& WindowSDL::create_notify(const NotifyCreateInfo& info)
    {
        create_system_notify(info);
        return *this;
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
        int_t s      = (channels == 4) ? 0 : 8;
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

            SDL_SetWindowIcon(_M_window, _M_icon);
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
                case WindowAttribute::Resizable:
                    SDL_SetWindowResizable(_M_window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttribute::FullScreen:
                    fullscreen_mode._M_flag       = value ? SDL_WINDOW_FULLSCREEN : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttribute::FullScreenDesktop:
                    fullscreen_mode._M_flag       = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttribute::Shown:
                {
                    value ? show() : hide();
                    break;
                }
                case WindowAttribute::Hidden:
                {
                    value ? hide() : show();
                    break;
                }
                case WindowAttribute::BorderLess:
                    SDL_SetWindowBordered(_M_window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttribute::InputFocus:
                {
                    if (value)
                        SDL_SetWindowInputFocus(_M_window);
                    break;
                }

                case WindowAttribute::Minimized:
                {
                    if (value)
                        SDL_MinimizeWindow(_M_window);
                    break;
                }

                case WindowAttribute::Maximized:
                {
                    if (value)
                        SDL_MaximizeWindow(_M_window);
                    break;
                }
                case WindowAttribute::MouseGrabbed:
                {
                    SDL_SetWindowMouseGrab(_M_window, static_cast<SDL_bool>(value));
                    break;
                }

                case WindowAttribute::KeyboardGrabbed:
                {
                    SDL_SetWindowKeyboardGrab(_M_window, static_cast<SDL_bool>(value));
                    break;
                }

                default:
                    break;
            }


            if (fullscreen_mode._M_fullscreen)
                SDL_SetWindowFullscreen(_M_window, fullscreen_mode._M_flag);
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
        if (_M_c_mode != mode)
        {
            SDL_ShowCursor((mode == CursorMode::Hidden ? SDL_DISABLE : SDL_ENABLE));
            _M_c_mode = mode;
        }
        return *this;
    }

    CursorMode WindowSDL::cursor_mode()
    {
        return _M_c_mode;
    }

    bool WindowSDL::support_orientation(WindowOrientation orientations)
    {
        //        static Map<WindowOrientation, const char*> _M_orientation_map = {
        //                {WindowOrientation::Landscape, "LandscapeRight"},
        //                {WindowOrientation::LandscapeFlipped, "LandscapeLeft"},
        //                {WindowOrientation::Portrait, "Portrait"},
        //                {WindowOrientation::PortraitFlipped, "PortraitUpsideDown"}};


        //        String result;
        //        for (auto ell : orientations)
        //        {
        //            if (!result.empty())
        //                result += " ";
        //            result += _M_orientation_map.at(ell);
        //        }

        //        SDL_SetHint(SDL_HINT_ORIENTATIONS, result.c_str());
        return false;
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
        while (SDL_PollEvent(&_M_event))
        {
            process_event();
        }

        return *this;
    }

    WindowInterface& WindowSDL::wait_for_events()
    {
        SDL_WaitEvent(&_M_event);
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
        button_event.clicks = _M_event.button.clicks;
        button_event.x      = _M_event.button.x;
        button_event.y      = _M_event.button.y;

        auto it = mouse_buttons.find(_M_event.button.button);
        if (it != mouse_buttons.end())
        {
            button_event.button = it->second;
        }

        if (_M_event.type == SDL_MOUSEBUTTONDOWN)
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
        for (auto func : _M_on_event)
        {
            func(&_M_event);
        }


        switch (_M_event.type)
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
                auto it = window_event_types.find(_M_event.window.event);
                if (it != window_event_types.end())
                {
                    WindowEvent engine_event;
                    engine_event.type = it->second;

                    engine_event.x = _M_event.window.data1;
                    engine_event.y = _M_event.window.data2;

                    new_event(Window, engine_event);
                }
                break;
            }

            case SDL_KEYDOWN:
            {
                KeyEvent key_event;
                auto it = keys.find(_M_event.key.keysym.scancode);
                if (it != keys.end())
                {
                    key_event.key    = it->second;
                    key_event.repeat = _M_event.key.repeat;
                    new_event(KeyDown, key_event);
                }
                else
                {
                    error_log("SDL Window System", "Cannot find scancode '%d'", _M_event.key.keysym.scancode);
                }
                break;
            }

            case SDL_KEYUP:
            {
                KeyEvent key_event;
                auto it = keys.find(_M_event.key.keysym.scancode);
                if (it != keys.end())
                {
                    key_event.key    = it->second;
                    key_event.repeat = _M_event.key.repeat;
                    new_event(KeyUp, key_event);
                }
                else
                {
                    error_log("SDL Window System", "Cannot find scancode '%d'", _M_event.key.keysym.scancode);
                }
                break;
            }


            case SDL_MOUSEMOTION:
            {
                MouseMotionEvent mouse_motion;
                mouse_motion.x    = _M_event.motion.x;
                mouse_motion.y    = height() - _M_event.motion.y;
                mouse_motion.xrel = _M_event.motion.xrel;
                mouse_motion.yrel = -_M_event.motion.yrel;

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
                wheel_event.x = _M_event.wheel.preciseX;
                wheel_event.y = _M_event.wheel.preciseY;
                wheel_event.direction =
                        _M_event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? Mouse::Normal : Mouse::Flipped;
                new_event(MouseWheel, wheel_event);
                break;
            }

            case SDL_CONTROLLERDEVICEADDED:
            {
                game_controllers[_M_event.cdevice.which] = SDL_GameControllerOpen(_M_event.cdevice.which);
                ControllerDeviceAddedEvent c_event;
                c_event.id = static_cast<Identifier>(_M_event.cdevice.which) + 1;
                new_event(ControllerDeviceAdded, c_event);
                break;
            }

            case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_GameControllerClose(game_controllers[_M_event.cdevice.which]);
                game_controllers.erase(_M_event.cdevice.which);
                ControllerDeviceAddedEvent c_event;
                c_event.id = static_cast<Identifier>(_M_event.cdevice.which) + 1;
                new_event(ControllerDeviceRemoved, c_event);
                break;
            }

            case SDL_CONTROLLERAXISMOTION:
            {
                ControllerAxisMotionEvent motion_event;
                motion_event.id = static_cast<Identifier>(_M_event.caxis.which) + 1;
                try
                {
                    motion_event.axis = axis_type.at(_M_event.caxis.axis);
                }
                catch (...)
                {
                    motion_event.axis = GameController::Axis::None;
                }

                motion_event.value = _M_event.caxis.value;
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
        if (_M_api == SDL_WINDOW_VULKAN)
        {
            va_list args;
            va_start(args, any_text);
            VkInstance instance = va_arg(args, VkInstance);
            va_end(args);

            SDL_Vulkan_CreateSurface(_M_window, instance, &_M_vulkan_surface);
            return &_M_vulkan_surface;
        }
        else if (_M_api == SDL_WINDOW_OPENGL)
        {
            _M_gl_context = SDL_GL_CreateContext(_M_window);
            if (!_M_gl_context)
            {
                throw std::runtime_error(SDL_GetError());
            }

            if (SDL_GL_MakeCurrent(_M_window, _M_gl_context) != 0)
            {
                error_log("SDL", "Cannot set context as current: %s", SDL_GetError());
            }

            return _M_gl_context;
        }

        return nullptr;
    }

    WindowInterface& WindowSDL::swap_buffers()
    {
        if (_M_api == SDL_WINDOW_OPENGL)
        {
            SDL_GL_SwapWindow(_M_window);
        }
        return *this;
    }

    Vector<const char*> WindowSDL::required_extensions()
    {
        if (_M_api == SDL_WINDOW_VULKAN)
        {
            uint32_t count = 0;

            SDL_Vulkan_GetInstanceExtensions(_M_window, &count, nullptr);
            Vector<const char*> extensions(count);
            SDL_Vulkan_GetInstanceExtensions(_M_window, &count, extensions.data());
            return extensions;
        }

        return {};
    }

    WindowInterface& WindowSDL::add_event_callback(Identifier system_id, const EventCallback& callback)
    {
        _M_event_callbacks[system_id].push_back(callback);
        return *this;
    }

    WindowInterface& WindowSDL::remove_all_callbacks(Identifier system_id)
    {
        _M_event_callbacks.erase(system_id);
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
        for (auto& system_callbacks : _M_event_callbacks)
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
        ImGui_ImplSDL2_InitForOpenGL(_M_window, _M_gl_context);
    }

    void WindowSDL::initialize_imgui_vulkan()
    {
        ImGui_ImplSDL2_InitForVulkan(_M_window);
    }

    WindowInterface& WindowSDL::initialize_imgui()
    {
        IMGUI_CHECKVERSION();
        _M_imgui_context = ImGui::CreateContext();


        switch (_M_api)
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

        _M_on_event.insert(process_imgui_event);
        return *this;
    }

    WindowInterface& WindowSDL::terminate_imgui()
    {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(_M_imgui_context);
        _M_on_event.erase(process_imgui_event);
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

    String WindowSDL::error() const
    {
        return SDL_GetError();
    }

    bool WindowSDL::has_error() const
    {
        const char* msg = SDL_GetError();
        return std::strcmp(msg, "") != 0;
    }

}// namespace Engine


extern "C" FORCE_ENGINE_EXPORT Engine::WindowInterface* create_library_interface()
{
    SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO);
    return new Engine::WindowSDL();
}
