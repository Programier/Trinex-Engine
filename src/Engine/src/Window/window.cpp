#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/shader_system.hpp>
#include <SDL.h>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <api.hpp>
#include <list>
#include <sdl_surface.hpp>
#include <vector>

using namespace Engine;


static Window window;
#define win_init_error throw std::runtime_error("Init window first")

#define sdl_window data._M_window
#define check_init(value)                                                                                              \
    if (!data._M_is_inited)                                                                                            \
    return value


static const std::unordered_map<WindowAttrib, SDL_WindowFlags> window_attributes = {
        {WindowAttrib::WIN_RESIZABLE, SDL_WINDOW_RESIZABLE},
        {WindowAttrib::WIN_FULLSCREEN, SDL_WINDOW_FULLSCREEN},
        {WindowAttrib::WIN_FULLSCREEN_DESKTOP, SDL_WINDOW_FULLSCREEN_DESKTOP},
        {WindowAttrib::WIN_SHOWN, SDL_WINDOW_SHOWN},
        {WindowAttrib::WIN_HIDDEN, SDL_WINDOW_HIDDEN},
        {WindowAttrib::WIN_BORDERLESS, SDL_WINDOW_BORDERLESS},
        {WindowAttrib::WIN_MOUSE_FOCUS, SDL_WINDOW_MOUSE_FOCUS},
        {WindowAttrib::WIN_INPUT_FOCUS, SDL_WINDOW_INPUT_FOCUS},
        {WindowAttrib::WIN_INPUT_GRABBED, SDL_WINDOW_INPUT_GRABBED},
        {WindowAttrib::WIN_MINIMIZED, SDL_WINDOW_MINIMIZED},
        {WindowAttrib::WIN_MAXIMIZED, SDL_WINDOW_MAXIMIZED},
        {WindowAttrib::WIN_MOUSE_CAPTURE, SDL_WINDOW_MOUSE_CAPTURE},
        {WindowAttrib::WIN_ALLOW_HIGHDPI, SDL_WINDOW_ALLOW_HIGHDPI},
        {WindowAttrib::WIN_MOUSE_GRABBED, SDL_WINDOW_MOUSE_GRABBED},
        {WindowAttrib::WIN_KEYBOARD_GRABBED, SDL_WINDOW_KEYBOARD_GRABBED}};


static const WindowAttrib attrib_list[] = {
        WIN_RESIZABLE,     WIN_FULLSCREEN,    WIN_FULLSCREEN_DESKTOP, WIN_SHOWN,
        WIN_HIDDEN,        WIN_BORDERLESS,    WIN_MOUSE_FOCUS,        WIN_INPUT_FOCUS,
        WIN_INPUT_GRABBED, WIN_MINIMIZED,     WIN_MAXIMIZED,          WIN_TRANSPARENT_FRAMEBUFFER,
        WIN_MOUSE_CAPTURE, WIN_ALLOW_HIGHDPI, WIN_MOUSE_GRABBED,      WIN_KEYBOARD_GRABBED};

static constexpr int_t attributes_count = sizeof(attrib_list) / sizeof(WindowAttrib);

static std::list<WindowAttrib> parse_win_attibutes(uint16_t attrib)
{
    std::list<WindowAttrib> attributes;
    for (int_t i = 0; i < attributes_count; i++)
    {
        if (attrib & (1 << i))
            attributes.push_back(attrib_list[i]);
    }
    return attributes;
}

static uint32_t to_sdl_attrib(const std::list<WindowAttrib>& attrib)
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


static struct WindowData {
    SDL_Window* _M_window = nullptr;
    SDL_GLContext _M_GL_context = nullptr;
    bool _M_is_inited = false;
    String _M_title;

    SizeLimits2D _M_limits;
    Size2D _M_size = {-1, -1};
    Size2D _M_position;

    int_t _M_swap_interval = 1;
    float _M_opacity = 0.f;

    std::vector<String> _M_dropped_paths;

    Color _M_background_color = Color(0, 0, 0, 1);
    Cursor _M_cursor;

    Image _M_icon;
    AspectRation _M_ration;
    bool _M_enable_ration = false;

    CursorMode _M_cursor_mode = CursorMode::NORMAL;
    const char* _M_X11_compositing = "0";
    SDL_Surface* _M_icon_surface = nullptr;
    bool _M_is_transparent_framebuffer = false;
    Uint32 _M_flags;
    bool _M_change_viewport_on_resize = true;
    std::size_t _M_objects = 0;
    std::size_t _M_frame = 0;
} data;


static void free_icon_surface()
{
    SDL_SetWindowIcon(sdl_window, nullptr);
    if (data._M_icon_surface)
    {
        logger->log("Window: Destroy icon surface\n");
        SDL_FreeSurface(data._M_icon_surface);
    }
    data._M_icon_surface = nullptr;
}

//          WINDOW INITIALIZATION


const Window& Window::init(float width, float height, const String& title, uint16_t attributes)
{
    if (data._M_is_inited)
        return window;

    if (!EngineInstance::get_instance()->is_inited())
        throw std::runtime_error("Window: Init Engine first");

    data._M_limits.max = Monitor::size();

    auto error = [](const std::string& msg = "") {
        logger->log("Window: Failed to create new window, error: '%s'\n", msg.c_str());
        Window::close();
        throw std::runtime_error("Window: Failed to create Window");
    };

    logger->log("Window: Creating new window '%ls'\n", title.c_str());

    uint32_t attrib = to_sdl_attrib(parse_win_attibutes(attributes));
    data._M_title = title;

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, data._M_X11_compositing);


    auto sdl_window_api =
            (EngineInstance::get_instance()->api() == EngineAPI::OpenGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN);

    sdl_window = SDL_CreateWindow(Strings::to_std_string(data._M_title).c_str(), SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, cast(int, width), cast(int, height),
                                  sdl_window_api | SDL_WINDOW_SHOWN | attrib);


    if (sdl_window == nullptr)
        error(SDL_GetError());

    data._M_GL_context = EngineInstance::get_instance()->api_interface()->init_window(sdl_window);
    if (data._M_GL_context == nullptr)
    {
        error("External error");
    }


    data._M_size = {width, height};
    data._M_ration = {width, height};
    window.update_view_port();

    Window::size_limits(data._M_limits);

    // Init shaders
    Engine::ShaderSystem::init();
    data._M_is_inited = true;

    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return window;
}

const Window& Window::init(const Size2D& size, const String& title, uint16_t attrib)
{
    return init(size.x, size.y, title, attrib);
}


//          CLOSING WINDOW


const Window& Window::close()
{
    check_init(window);
    free_icon_surface();
    logger->log("Closing window\n");

    EngineInstance::get_instance()->api_interface()->destroy_window();

    if (data._M_window)
        SDL_DestroyWindow(data._M_window);

    logger->log("Engine::Window: Window '%ls' closed\n", data._M_title.c_str());
    data = WindowData();

    return window;
}


bool Window::is_open()
{
    return data._M_is_inited;
}


const Window& Window::swap_buffers()
{
    check_init(window);
    EngineInstance::get_instance()->api_interface()->swap_buffer(data._M_window);
    return window;
}


//          CHANGING SIZE OF WINDOW

Size1D Window::width()
{
    return data._M_size.x;
}

const Window& Window::width(const Size1D& width)
{
    check_init(window);
    SDL_SetWindowSize(sdl_window, cast(int, width), cast(int, data._M_size.y));
    return window;
}

Size1D Window::height()
{
    return data._M_size.y;
}

const Window& Window::height(const Size1D& height)
{
    check_init(window);
    SDL_SetWindowSize(sdl_window, cast(int, data._M_size.x), cast(int, height));
    return window;
}

const Size2D& Window::size()
{
    return data._M_size;
}

const Window& Window::size(const Size2D& size)
{
    check_init(window);
    SDL_SetWindowSize(sdl_window, cast(int, size.x), cast(int, size.y));
    return window;
}


namespace Engine
{
    void process_window_event(SDL_WindowEvent& event)
    {
        ++data._M_frame;
        switch (event.event)
        {
            case SDL_WINDOWEVENT_MOVED:
            {
                data._M_position = {event.data1, event.data2};
                break;
            }

            case SDL_WINDOWEVENT_SIZE_CHANGED:
            case SDL_WINDOWEVENT_RESIZED:
            {
                data._M_size = {event.data1, event.data2};
                if (data._M_change_viewport_on_resize)
                    window.update_view_port();
                Window::on_resize.trigger();
                break;
            }

            case SDL_WINDOWEVENT_ENTER:
            {
                break;
            }


            case SDL_WINDOWEVENT_CLOSE:
                window.close();
                break;
        }
    }


    void process_paths_event(SDL_DropEvent& event)
    {
        if (event.file)
        {
            data._M_dropped_paths.push_back(Strings::to_string(event.file));
            SDL_free(event.file);
        }
    }
}// namespace Engine

//          VSYNC


bool Window::vsync()
{
    return data._M_swap_interval > 0;
}

const Window& Window::vsync(const bool& value)
{
    data._M_swap_interval = cast(int, value);
    return swap_interval(cast(int, value));
}

int_t Window::swap_interval()
{
    return data._M_swap_interval;
}

const Window& Window::swap_interval(int_t value)
{
    check_init(window);
    data._M_swap_interval = value;
    EngineInstance::get_instance()->api_interface()->swap_interval(value);
    return window;
}


//          WINDOW TITLE

const String& Window::title()
{
    return data._M_title;
}

const Window& Window::title(const String& title)
{

    check_init(window);
    data._M_title = title;
    SDL_SetWindowTitle(sdl_window, Strings::to_std_string(data._M_title).c_str());
    return window;
}


//          WINDOW POSITION

const Point2D& Window::position()
{
    return data._M_position;
}


const Window& Window::position(const Point2D& position)
{
    if (data._M_is_inited)
        SDL_SetWindowPosition(sdl_window, cast(int, position.x), cast(int, position.y));
    return window;
}


//          DROPPED PATHS
const std::vector<String>& Window::dropped_paths()
{
    return data._M_dropped_paths;
}


const Window& Window::clear_dropped_paths()
{
    data._M_dropped_paths.clear();
    return window;
}

//      REZISABLE WINDOW

bool Window::rezisable()
{
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return cast(bool, data._M_flags& SDL_WINDOW_RESIZABLE);
}

const Window& Window::rezisable(bool value)
{
    check_init(window);
    SDL_SetWindowResizable(sdl_window, cast(SDL_bool, value));
    return window;
}

//  FOCUS WINDOW

const Window& Window::focus()
{
    check_init(window);
    SDL_SetWindowInputFocus(sdl_window);
    return window;
}

bool Window::focused()
{
    check_init(false);
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return cast(bool, data._M_flags& SDL_WINDOW_INPUT_FOCUS);
}

Color& Window::background_color()
{
    return data._M_background_color;
}

const Window& Window::background_color(const Color& color)
{
    check_init(window);
    data._M_background_color = color;
    EngineInstance::get_instance()->api_interface()->clear_color(color);
    return window;
}


//      OTHER FUNCTIONS

const Window& Window::show()
{
    check_init(window);
    SDL_ShowWindow(sdl_window);
    return window;
}

const Window& Window::hide()
{
    check_init(window);
    SDL_HideWindow(sdl_window);
    return window;
}

bool Window::is_visible()
{
    check_init(false);
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return cast(bool, data._M_flags& SDL_WINDOW_SHOWN);
}


const Window& Window::clear_buffer(const BufferType& buffer)
{
    check_init(window);
    window.framebuffer()->clear_buffer(buffer);
    return window;
}

bool Window::is_iconify()
{
    check_init(false);
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return cast(bool, data._M_flags& SDL_WINDOW_MINIMIZED);
}

const Window& Window::iconify()
{
    check_init(window);
    SDL_MinimizeWindow(sdl_window);
    return window;
}

bool Window::is_restored()
{
    check_init(false);
    return !is_iconify();
}

const Window& Window::restore()
{
    check_init(window);
    SDL_RestoreWindow(sdl_window);
    return window;
}


const Window& Window::opacity(float value)
{
    check_init(window);
    SDL_SetWindowOpacity(sdl_window, value);
    return window;
}

float Window::opacity()
{
    check_init(0.f);
    SDL_GetWindowOpacity(sdl_window, &data._M_opacity);
    return data._M_opacity;
}


const Window& Window::size_limits(const SizeLimits2D& limits)
{
    check_init(window);
    data._M_limits = limits;
    SDL_SetWindowMaximumSize(sdl_window, cast(int, data._M_limits.max.x), cast(int, data._M_limits.max.y));
    SDL_SetWindowMinimumSize(sdl_window, cast(int, data._M_limits.min.x), cast(int, data._M_limits.min.y));
    return window;
}

const SizeLimits2D& size_limits()
{
    return data._M_limits;
}

const Cursor& Window::cursor()
{
    return data._M_cursor;
}

const Window& Window::cursor(const Cursor& cursor)
{
    data._M_cursor = cursor;
    SDL_SetCursor(static_cast<SDL_Cursor*>(data._M_cursor.sdl_cursor()));
    return window;
}

const Window& Window::icon(const Image& image)
{
    check_init(window);
    data._M_icon = image;
    data._M_icon.add_alpha_channel();
    free_icon_surface();
    data._M_icon_surface = create_sdl_surface(data._M_icon);
    SDL_SetWindowIcon(sdl_window, data._M_icon_surface);
    return window;
}

const Window& Window::icon(const String& image)
{
    check_init(window);
    data._M_icon.load(image);
    free_icon_surface();
    data._M_icon_surface = create_sdl_surface(data._M_icon);
    SDL_SetWindowIcon(sdl_window, data._M_icon_surface);
    return window;
}

const Image& Window::icon()
{
    return data._M_icon;
}

const Window& Window::aspect_ration(const AspectRation& ration)
{
    check_init(window);
    data._M_ration = (data._M_enable_ration = (ration.x * ration.y > 0)) ? ration : AspectRation();
    return window;
}

const AspectRation& Window::aspect_ration()
{
    return data._M_ration;
}

const Window& Window::attribute(const WindowAttrib& attrib, bool value)
{
    check_init(window);
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    try
    {
        auto list = parse_win_attibutes(attrib);

        static struct {
            bool _M_fullscreen = false;
            Uint32 _M_flag = 0;
        } fullscreen_mode;

        fullscreen_mode._M_fullscreen = false;
        fullscreen_mode._M_flag = 0;

        for (auto& attrib : list)
        {

            switch (attrib)
            {
                case WindowAttrib::WIN_RESIZABLE:
                    SDL_SetWindowResizable(sdl_window, cast(SDL_bool, value));
                    break;

                case WindowAttrib::WIN_FULLSCREEN:
                    fullscreen_mode._M_flag = value ? SDL_WINDOW_FULLSCREEN : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttrib::WIN_FULLSCREEN_DESKTOP:
                    fullscreen_mode._M_flag = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttrib::WIN_SHOWN:
                {
                    value ? show() : hide();
                    break;
                }
                case WindowAttrib::WIN_HIDDEN:
                {
                    value ? hide() : show();
                    break;
                }
                case WindowAttrib::WIN_BORDERLESS:
                    SDL_SetWindowBordered(sdl_window, cast(SDL_bool, value));
                    break;

                case WindowAttrib::WIN_INPUT_FOCUS:
                {
                    if (value)
                        SDL_SetWindowInputFocus(sdl_window);
                    break;
                }

                case WindowAttrib::WIN_MINIMIZED:
                {
                    if (value)
                        SDL_MinimizeWindow(sdl_window);
                    break;
                }

                case WindowAttrib::WIN_MAXIMIZED:
                {
                    if (value)
                        SDL_MaximizeWindow(sdl_window);
                    break;
                }
                case WindowAttrib::WIN_MOUSE_GRABBED:
                {
                    SDL_SetWindowMouseGrab(sdl_window, cast(SDL_bool, value));
                    break;
                }

                case WindowAttrib::WIN_KEYBOARD_GRABBED:
                {
                    SDL_SetWindowKeyboardGrab(sdl_window, cast(SDL_bool, value));
                    break;
                }

                case WindowAttrib::WIN_TRANSPARENT_FRAMEBUFFER:
                {
                    if (data._M_is_inited)
                        logger->log("Window: Cannot change flag WIN_TRANSPARENT_FRAMEBUFFER after creating window\n");
                    else
                        data._M_is_transparent_framebuffer = value;
                    break;
                }

                default:
                    break;
            }
        }

        if (fullscreen_mode._M_fullscreen)
            SDL_SetWindowFullscreen(sdl_window, fullscreen_mode._M_flag);
    }
    catch (const std::exception& e)
    {
        logger->log("Window: %s\n", e.what());
    }

    return window;
}

bool Window::attribute(const WindowAttrib& attrib)
{
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    check_init(false);
    try
    {
        auto attribute_list = parse_win_attibutes(attrib);
        if (attribute_list.size() != 1)
            throw std::runtime_error("Failed to get attibute");
        auto attrib = attribute_list.front();
        if (attrib == WindowAttrib::WIN_TRANSPARENT_FRAMEBUFFER)
            return data._M_is_transparent_framebuffer;
        return cast(bool, data._M_flags& window_attributes.at(attrib));
    }
    catch (const std::exception& e)
    {
        logger->log("%s\n", e.what());
        return false;
    }
}


const Window& Window::X11_compositing(bool value)
{
    data._M_X11_compositing = (value ? "0" : "1");
    return window;
}


void* Window::SDL()
{
    return static_cast<void*>(data._M_window);
}

void* Window::SDL_OpenGL_context()
{
    return data._M_GL_context;
}


const Window& Window::cursor_mode(const CursorMode& mode)
{
    if (SDL_ShowCursor((mode == CursorMode::HIDDEN ? SDL_DISABLE : SDL_ENABLE)) < 0)
        Engine::logger->log(SDL_GetError());
    data._M_cursor_mode = mode;
    return window;
}

const CursorMode& Window::cursor_mode()
{
    return data._M_cursor_mode;
}


const Window& Window::bind()
{
    check_init(window);
    framebuffer()->bind();
    return window;
}

const Window& Window::update_view_port()
{
    check_init(window);
    framebuffer()->view_port(Point2D(), size());
    return window;
}

const Window& Window::set_orientation(uint_t orientation)
{
    static std::unordered_map<WindowOrientation, const char*> _M_orientation_map = {
            {WindowOrientation::WIN_ORIENTATION_LANDSCAPE, "LandscapeRight"},
            {WindowOrientation::WIN_ORIENTATION_LANDSCAPE_FLIPPED, "LandscapeLeft"},
            {WindowOrientation::WIN_ORIENTATION_PORTRAIT, "Portrait"},
            {WindowOrientation::WIN_ORIENTATION_PORTRAIT_FLIPPED, "PortraitUpsideDown"}};

    static WindowOrientation orientations[] = {WIN_ORIENTATION_PORTRAIT, WIN_ORIENTATION_PORTRAIT_FLIPPED,
                                               WIN_ORIENTATION_LANDSCAPE, WIN_ORIENTATION_LANDSCAPE_FLIPPED};

    if (data._M_is_inited)
    {
        logger->log("Window: Can't change orientation after creating window\n");
        return window;
    }

    std::string result;
    for (auto ell : orientations)
    {
        if (orientation & ell)
        {
            if (!result.empty())
                result += " ";
            result += _M_orientation_map.at(ell);
        }
    }

    SDL_SetHint(SDL_HINT_ORIENTATIONS, result.c_str());
    return window;
}

const Window& Window::start_text_input()
{
    SDL_StartTextInput();
    return window;
}

const Window& Window::stop_text_input()
{
    SDL_StopTextInput();
    return window;
}

const Window& Window::update_viewport_on_resize(bool value)
{
    data._M_change_viewport_on_resize = value;
    return window;
}

bool Window::update_viewport_on_resize()
{
    return data._M_change_viewport_on_resize;
}

CallBacks<void> Window::on_resize;


std::size_t Window::frame_number()
{
    return data._M_frame;
}

BasicFrameBuffer* Window::framebuffer()
{
    return window.instance_cast<BasicFrameBuffer>();
}


// Window constructors

Window::Window()
{
    data._M_objects++;
}

Window::Window(float width, float height, const String& title, uint16_t attrib) : Window()
{
    init(width, height, title, attrib);
}

Window::Window(const Size2D& size, const String& title, uint16_t attrib) : Window()
{
    init(size.x, size.y, title, attrib);
}


Window::Window(const Window&) : Window()
{}


Window& Window::operator=(const Window&)
{
    data._M_objects++;
    return *this;
}

Window::~Window()
{
    data._M_objects--;
    if (!data._M_objects)
        this->close();
}
