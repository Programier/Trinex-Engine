#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Image/image.hpp>
#include <SDL_gamecontroller.h>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <window.hpp>


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


#define has_flag(flag) static_cast<bool>(SDL_GetWindowFlags(m_window) & flag)
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


    static int validate_pos(int pos)
    {
        return pos < 0 ? SDL_WINDOWPOS_UNDEFINED : pos;
    }

    WindowInterface* WindowSDL::initialize(const WindowConfig* info)
    {
        //m_vsync_status = info->vsync;
        uint32_t attrib = to_sdl_attrib(info->attributes);

        if ((attrib & SDL_WINDOW_SHOWN) != SDL_WINDOW_SHOWN && (attrib & SDL_WINDOW_HIDDEN) != SDL_WINDOW_HIDDEN)
        {
            attrib |= SDL_WINDOW_SHOWN;
        }

#if PLATFORM_LINUX
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

        m_api = sdl_api(info->api_name);

        m_window = SDL_CreateWindow(info->title.c_str(), validate_pos(info->position.x), validate_pos(info->position.y),
                                    static_cast<int>(info->size.x), static_cast<int>(info->size.y),
                                    m_api | SDL_WINDOW_ALLOW_HIGHDPI | attrib);

        m_id = static_cast<Identifier>(SDL_GetWindowID(m_window));

        if (m_window == nullptr)
            window_initialize_error(SDL_GetError());
        else
        {
            if (m_api == SDL_WINDOW_OPENGL)
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

                vsync(info->vsync);
            }
            else if (m_api == SDL_WINDOW_VULKAN)
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
        SDL_GetWindowSize(m_window, &w, &h);
        return {w, h};
    }

    WindowInterface& WindowSDL::size(const Size2D& size)
    {
        SDL_SetWindowSize(m_window, size.x, size.y);
        return *this;
    }

    String WindowSDL::title()
    {
        return SDL_GetWindowTitle(m_window);
    }

    WindowInterface& WindowSDL::title(const String& title)
    {
        SDL_SetWindowTitle(m_window, title.c_str());
        return *this;
    }

    Point2D WindowSDL::position()
    {
        int x, y;
        SDL_GetWindowPosition(m_window, &x, &y);
        return {x, y};
    }

    WindowInterface& WindowSDL::position(const Point2D& position)
    {
        SDL_SetWindowPosition(m_window, position.x, position.y);
        return *this;
    }

    bool WindowSDL::resizable()
    {
        return has_flag(SDL_WINDOW_RESIZABLE);
    }

    WindowInterface& WindowSDL::resizable(bool value)
    {
        SDL_SetWindowResizable(m_window, static_cast<SDL_bool>(value));
        return *this;
    }

    WindowInterface& WindowSDL::focus()
    {
        SDL_SetWindowInputFocus(m_window);
        return *this;
    }

    bool WindowSDL::focused()
    {
        return has_flag(SDL_WINDOW_INPUT_FOCUS);
    }

    WindowInterface& WindowSDL::show()
    {
        SDL_ShowWindow(m_window);
        return *this;
    }

    WindowInterface& WindowSDL::hide()
    {
        SDL_HideWindow(m_window);
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
        SDL_MinimizeWindow(m_window);
        return *this;
    }

    bool WindowSDL::is_restored()
    {
        return !is_iconify();
    }

    WindowInterface& WindowSDL::restore()
    {
        SDL_RestoreWindow(m_window);
        return *this;
    }

    WindowInterface& WindowSDL::opacity(float value)
    {
        SDL_SetWindowOpacity(m_window, value);
        return *this;
    }

    float WindowSDL::opacity()
    {
        float o;
        SDL_GetWindowOpacity(m_window, &o);
        return o;
    }

    WindowInterface& WindowSDL::size_limits(const SizeLimits2D& limits)
    {
        SDL_SetWindowMaximumSize(m_window, static_cast<int>(limits.max.x), static_cast<int>(limits.max.y));
        SDL_SetWindowMinimumSize(m_window, static_cast<int>(limits.min.x), static_cast<int>(limits.min.y));
        return *this;
    }

    SizeLimits2D WindowSDL::size_limits()
    {
        SizeLimits2D limits;
        int w, h;

        SDL_GetWindowMaximumSize(m_window, &w, &h);
        limits.max = {w, h};

        SDL_GetWindowMinimumSize(m_window, &w, &h);
        limits.min = {w, h};

        return limits;
    }

    WindowSDL& WindowSDL::vsync(bool flag)
    {
        if (m_api == SDL_WINDOW_OPENGL)
        {
            SDL_GL_SetSwapInterval(static_cast<int>(flag));
        }
        return *this;
    }

    bool WindowSDL::vsync()
    {
        if (m_api == SDL_WINDOW_OPENGL)
        {
            return SDL_GL_GetSwapInterval() != 0;
        }

        return false;
    }

    int_t WindowSDL::create_message_box(const MessageBoxCreateInfo& info)
    {
        SDL_MessageBoxData data;
        data.window  = m_window;
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

        sheme.colors[SDL_MESSAGEBOX_COLOR_TEXT] = {static_cast<Uint8>(info.sheme.text.r), static_cast<Uint8>(info.sheme.text.r),
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
        if (m_icon)
        {
            SDL_FreeSurface(m_icon);

            m_icon = nullptr;
        }
    }

    void WindowSDL::destroy_cursor()
    {
        if (m_cursor)
        {
            SDL_FreeCursor(m_cursor);
            m_cursor = nullptr;
        }

        if (m_cursor_icon)
        {
            SDL_FreeSurface(m_cursor_icon);
            m_cursor_icon = nullptr;
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
        SDL_Surface* surface =
                SDL_CreateRGBSurfaceFrom(data, width, height, channels * 8, width * channels, r_mask, g_mask, b_mask, a_mask);
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

            m_icon_buffer = image.buffer();
            m_icon        = create_surface(m_icon_buffer, image.width(), image.height(), channels);

            SDL_SetWindowIcon(m_window, m_icon);
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

            m_cursor_icon_buffer = image.buffer();
            m_cursor_icon        = create_surface(m_cursor_icon_buffer, image.width(), image.height(), channels);

            if (m_cursor_icon)
            {
                m_cursor = SDL_CreateColorCursor(m_cursor_icon, hotspot.x, hotspot.y);
            }

            SDL_SetCursor(m_cursor);
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
                bool m_fullscreen = false;
                Uint32 m_flag     = 0;
            } fullscreen_mode;

            fullscreen_mode.m_fullscreen = false;
            fullscreen_mode.m_flag       = 0;


            switch (attrib)
            {
                case WindowAttribute::Resizable:
                    SDL_SetWindowResizable(m_window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttribute::FullScreen:
                    fullscreen_mode.m_flag       = value ? SDL_WINDOW_FULLSCREEN : 0;
                    fullscreen_mode.m_fullscreen = true;
                    break;

                case WindowAttribute::FullScreenDesktop:
                    fullscreen_mode.m_flag       = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                    fullscreen_mode.m_fullscreen = true;
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
                    SDL_SetWindowBordered(m_window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttribute::InputFocus:
                {
                    if (value)
                        SDL_SetWindowInputFocus(m_window);
                    break;
                }

                case WindowAttribute::Minimized:
                {
                    if (value)
                        SDL_MinimizeWindow(m_window);
                    break;
                }

                case WindowAttribute::Maximized:
                {
                    if (value)
                        SDL_MaximizeWindow(m_window);
                    break;
                }
                case WindowAttribute::MouseGrabbed:
                {
                    SDL_SetWindowMouseGrab(m_window, static_cast<SDL_bool>(value));
                    break;
                }

                case WindowAttribute::KeyboardGrabbed:
                {
                    SDL_SetWindowKeyboardGrab(m_window, static_cast<SDL_bool>(value));
                    break;
                }

                default:
                    break;
            }


            if (fullscreen_mode.m_fullscreen)
                SDL_SetWindowFullscreen(m_window, fullscreen_mode.m_flag);
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
        if (m_c_mode != mode)
        {
            SDL_ShowCursor((mode == CursorMode::Hidden ? SDL_DISABLE : SDL_ENABLE));
            m_c_mode = mode;
        }
        return *this;
    }

    CursorMode WindowSDL::cursor_mode()
    {
        return m_c_mode;
    }

    bool WindowSDL::support_orientation(WindowOrientation orientations)
    {
        //        static Map<WindowOrientation, const char*> m_orientation_map = {
        //                {WindowOrientation::Landscape, "LandscapeRight"},
        //                {WindowOrientation::LandscapeFlipped, "LandscapeLeft"},
        //                {WindowOrientation::Portrait, "Portrait"},
        //                {WindowOrientation::PortraitFlipped, "PortraitUpsideDown"}};


        //        String result;
        //        for (auto ell : orientations)
        //        {
        //            if (!result.empty())
        //                result += " ";
        //            result += m_orientation_map.at(ell);
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

    void* WindowSDL::create_api_context(const char* any_text, ...)
    {
        if (m_api == SDL_WINDOW_VULKAN)
        {
            if (!created_context.vulkan_surface)
            {
                va_list args;
                va_start(args, any_text);
                VkInstance instance = va_arg(args, VkInstance);
                va_end(args);

                SDL_Vulkan_CreateSurface(m_window, instance, &created_context.vulkan_surface);
            }
            binded_context.vulkan_surface = created_context.vulkan_surface;
            return &created_context.vulkan_surface;
        }
        else if (m_api == SDL_WINDOW_OPENGL)
        {
            if (!created_context.opengl_context)
            {
                SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
                created_context.opengl_context = SDL_GL_CreateContext(m_window);
                if (!created_context.opengl_context)
                {
                    throw EngineException(SDL_GetError());
                }
            }
            binded_context.opengl_context = created_context.opengl_context;
            return created_context.opengl_context;
        }

        return nullptr;
    }

    void WindowSDL::bind_api_context(void* context)
    {
        if (m_api == SDL_WINDOW_OPENGL)
        {
            binded_context.opengl_context = context;
        }
        else if (m_api == SDL_WINDOW_VULKAN)
        {
            binded_context.vulkan_surface = *reinterpret_cast<VkSurfaceKHR*>(context);
        }
    }

    WindowInterface& WindowSDL::make_current()
    {
        if (m_api == SDL_WINDOW_OPENGL && binded_context.opengl_context)
        {
            if (SDL_GL_MakeCurrent(m_window, binded_context.opengl_context) != 0)
            {
                error_log("SDL", "Cannot set context as current: %s", SDL_GetError());
            }
        }

        return *this;
    }

    WindowInterface& WindowSDL::destroy_api_context()
    {
        if (m_api == SDL_WINDOW_OPENGL && created_context.opengl_context)
        {
            SDL_GL_DeleteContext(created_context.opengl_context);
            created_context.opengl_context = nullptr;
            return *this;
        }
        return *this;
    }

    WindowInterface& WindowSDL::present()
    {
        if (m_api == SDL_WINDOW_OPENGL)
        {
            SDL_GL_SwapWindow(m_window);
        }
        return *this;
    }

    Vector<const char*> WindowSDL::required_extensions()
    {
        if (m_api == SDL_WINDOW_VULKAN)
        {
            uint32_t count = 0;

            SDL_Vulkan_GetInstanceExtensions(m_window, &count, nullptr);
            Vector<const char*> extensions(count);
            SDL_Vulkan_GetInstanceExtensions(m_window, &count, extensions.data());
            return extensions;
        }

        return {};
    }

    Identifier WindowSDL::id()
    {
        return m_id;
    }


    void WindowSDL::initialize_imgui_opengl()
    {
        ImGui_ImplSDL2_InitForOpenGL(m_window, binded_context.opengl_context);
    }

    void WindowSDL::initialize_imgui_vulkan()
    {
        ImGui_ImplSDL2_InitForVulkan(m_window);
    }

    WindowInterface& WindowSDL::initialize_imgui()
    {
        SDL_SetWindowData(m_window, "trinex_imgui_context", ImGui::GetCurrentContext());

        switch (m_api)
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
        SDL_SetWindowData(m_window, "trinex_imgui_context", nullptr);
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
        if (m_window)
        {
            destroy_api_context();
            destroy_icon();
            destroy_cursor();
            SDL_DestroyWindow(m_window);
            m_window = 0;
        }
    }
}// namespace Engine


void* create_engine_window(SDL_Window* _main_window, SDL_Window* window, ImGuiViewport* viewport)
{
    Engine::WindowSDL* main_window     = nullptr;
    Engine::Window* main_window_handle = nullptr;
    {
        main_window_handle = Engine::WindowManager::instance()->find(SDL_GetWindowID(_main_window));
        if (!main_window_handle)
            return nullptr;
        main_window = reinterpret_cast<Engine::WindowSDL*>(main_window_handle->interface());
        if (!main_window)
            return nullptr;
    }

    SDL_SetWindowData(window, "trinex_imgui_context", ImGui::GetCurrentContext());

    Engine::WindowSDL* new_window = new Engine::WindowSDL();
    new_window->m_window          = window;
    new_window->m_id              = static_cast<Engine::Identifier>(SDL_GetWindowID(window));
    new_window->m_api             = main_window->m_api;


    Engine::WindowConfig config;
    config.client = "Engine::ImGuiViewportClient";
    config.vsync  = main_window_handle->render_viewport()->vsync();
    auto* client  = reinterpret_cast<Engine::ImGuiRenderer::ImGuiViewportClient*>(
            Engine::WindowManager::instance()->create_window(config, nullptr, new_window)->render_viewport()->client());
    if (client)
    {
        client->viewport = viewport;
    }

    return new_window->binded_context.opengl_context;
}

void destroy_engine_window(SDL_Window* window)
{
    Engine::Window* main_window_handle = Engine::WindowManager::instance()->find(SDL_GetWindowID(window));
    Engine::WindowManager::instance()->destroy_window(main_window_handle);
}
