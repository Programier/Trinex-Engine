#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Image/image.hpp>
#include <SDL_gamecontroller.h>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
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
        if (api_name == "OpenGL" || api_name == "OpenGLES")
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

    WindowInterface* WindowSDL::initialize(const WindowConfig* info)
    {
        _M_vsync_status = info->vsync;
        uint32_t attrib = to_sdl_attrib(info->attributes);
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

        _M_api = sdl_api(info->api_name);

        _M_window = SDL_CreateWindow(info->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     static_cast<int>(info->size.x), static_cast<int>(info->size.y),
                                     _M_api | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | attrib);

        _M_id = static_cast<Identifier>(SDL_GetWindowID(_M_window));

        if (_M_window == nullptr)
            window_initialize_error(SDL_GetError());
        else
        {
            if (_M_api == SDL_WINDOW_OPENGL)
            {

#if !PLATFORM_ANDROID
                if (!info->api_name.ends_with("GLES"))
                {
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                }
                else
#endif
                {
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
                }
            }
            else if (_M_api == SDL_WINDOW_VULKAN)
            {
            }
        }

        return this;
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

    template<typename Type>
    using ValueMap = const Map<Uint8, Type>;


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
            void* gl_context = SDL_GL_CreateContext(_M_window);
            if (!gl_context)
            {
                throw EngineException(SDL_GetError());
            }

            make_current(gl_context);
            vsync(_M_vsync_status);
            return gl_context;
        }

        return nullptr;
    }

    WindowInterface& WindowSDL::make_current(void* context)
    {
        if (_M_api == SDL_WINDOW_OPENGL)
        {
            if (SDL_GL_MakeCurrent(_M_window, context) != 0)
            {
                error_log("SDL", "Cannot set context as current: %s", SDL_GetError());
            }
        }

        return *this;
    }

    WindowInterface& WindowSDL::destroy_surface(void* surface)
    {
        if (_M_api == SDL_WINDOW_OPENGL)
        {
            SDL_GL_DeleteContext(surface);
            return *this;
        }

        throw EngineException("Surface must be destroyed by Graphical API!");
        return *this;
    }

    WindowInterface& WindowSDL::link_surface(void* surface)
    {
        if (_M_api == SDL_WINDOW_OPENGL)
        {
            _M_gl_context = surface;
        }
        return *this;
    }

    WindowInterface& WindowSDL::present()
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

    Identifier WindowSDL::id()
    {
        return _M_id;
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
        return *this;
    }

    WindowInterface& WindowSDL::terminate_imgui()
    {
        ImGui_ImplSDL2_Shutdown();
        return *this;
    }

    WindowInterface& WindowSDL::new_imgui_frame()
    {
        ImGui_ImplSDL2_NewFrame();
        return *this;
    }

    WindowSDL::~WindowSDL()
    {
        if (_M_window)
        {
            destroy_icon();
            destroy_cursor();
            SDL_DestroyWindow(_M_window);
            _M_window = 0;
        }
    }
}// namespace Engine
