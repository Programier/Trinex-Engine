#include <Graphics/shader_system.hpp>
#include <Init/init.hpp>
#include <SDL.h>
#include <Window/keyboard.hpp>
#include <Window/window.hpp>
#include <iostream>
#include <list>
#include <opengl.hpp>
#include <sdl_surface.hpp>
#include <vector>

using namespace Engine;


static Window window;
#define win_init_error throw std::runtime_error("Init window first")

#define sdl_window data._M_window
#define check_init(value)                                                                                                   \
    if (!data._M_is_inited)                                                                                                 \
    return value

const EngineAPI& api = Engine::API();


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

static constexpr int attributes_count = sizeof(attrib_list) / sizeof(WindowAttrib);

std::list<WindowAttrib> parse_win_attibutes(uint16_t attrib)
{
    std::list<WindowAttrib> attributes;
    for (int i = 0; i < attributes_count; i++)
    {
        if (attrib & (1 << i))
            attributes.push_back(attrib_list[i]);
    }
    return attributes;
}

uint32_t to_sdl_attrib(const std::list<WindowAttrib>& attrib)
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

uint32_t to_sdl_attrib(WindowAttrib attrib)
{
    return to_sdl_attrib(parse_win_attibutes(attrib));
}


static struct WindowData {
    SDL_Window* _M_window = nullptr;
    SDL_GLContext _M_GL_context = nullptr;

    bool _M_is_inited = false;
    std::string _M_title;

    SizeLimits2D _M_limits;
    Size2D _M_size = {-1, -1};
    Size2D _M_position;

    int _M_swap_interval = 1;
    float _M_opacity = 0.f;

    std::vector<std::string> _M_dropped_paths;

    Color _M_background_color = Color(0, 0, 0, 1);
    Cursor _M_cursor;

    Image _M_icon;
    AspectRation _M_ration;
    bool _M_enable_ration = false;

    std::vector<int> _M_backup;
    CursorMode _M_cursor_mode = CursorMode::NORMAL;
    const char* _M_X11_compositing = "0";
    SDL_Surface* _M_icon_surface = nullptr;
    bool _M_is_transparent_framebuffer = false;
    Uint32 _M_flags;
    bool _M_change_viewport_on_resize = true;
    SDL_Event event;
    std::vector<void (*)(void*)> _M_event_callback;

    struct Keys {
        std::vector<KeyStatus> _M_keys;
        Key _M_last_key = Key::KEY_UNKNOWN;
        Key _M_last_released = Key::KEY_UNKNOWN;

        Key _M_last_mouse_key = Key::KEY_UNKNOWN;
        Key _M_last_mouse_released = Key::KEY_UNKNOWN;

        std::list<Key> _M_last_evented_keys;

        Keys()
        {
            _M_keys.resize(key_count());
        }

    } keys;

    std::size_t _M_time = .0f;
    std::size_t _M_diff_time = .0f;
    std::size_t _M_last_time = .0f;

    unsigned int _M_last_symbol = 0;

    Point2D _M_mouse_position;
    Offset2D _M_mouse_offset;
    Offset2D _M_scroll_offset;

    // Callbacks
    struct {
        std::function<void(const Size2D&)> _M_resize_callback = nullptr;
        std::function<void(const Point2D&)> _M_position_callback = nullptr;
        std::function<void(int, const char*[])> _M_dropped_paths_callback = nullptr;
    } callbacks;

    std::size_t _M_objects = 0;
} data;


static void free_icon_surface()
{
    SDL_SetWindowIcon(sdl_window, nullptr);
    if (data._M_icon_surface)
    {
        std::clog << "Window: Destroy icon surface" << std::endl;
        SDL_FreeSurface(data._M_icon_surface);
    }
    data._M_icon_surface = nullptr;
}


//          CALLBACKS


//      RESIZE CALLBACK
static void OpenGL_viewport_resize()
{
    glViewport(0, 0, cast(GLsizei, data._M_size.x), cast(GLsizei, data._M_size.y));
}

static void Vulkan_viewport_resize()
{
    throw not_implemented;
}

static void (*win_viewport_resize[])() = {OpenGL_viewport_resize, Vulkan_viewport_resize};


static void event_preprocessing()
{
    // Keyboard
    for (auto key : data.keys._M_last_evented_keys)
    {
        if (data.keys._M_keys[key] == KeyStatus::JUST_PRESSED)
            data.keys._M_keys[key] = KeyStatus::PRESSED;
        if (data.keys._M_keys[key] == KeyStatus::JUST_RELEASED)
            data.keys._M_keys[key] = KeyStatus::RELEASED;
    }

    data.keys._M_last_evented_keys.clear();


    data._M_scroll_offset = {0, 0};
    data._M_mouse_offset = {0, 0};

    data._M_time = SDL_GetTicks64();
    data._M_diff_time = data._M_time - data._M_last_time;
    data._M_last_time = data._M_time;
}


//      ENGINE CALLBACKS

std::function<void(const Size2D&)>& Window::Callbacks::resize_callback()
{
    return data.callbacks._M_resize_callback;
}

std::function<void(const Point2D&)>& Window::Callbacks::position_callback()
{
    return data.callbacks._M_position_callback;
}


//          WINDOW INITIALIZATION

static const Window& OpenGL_window_init(float width, float height, const std::string& title,
                                        const std::list<WindowAttrib>& attribs)
{
    data._M_limits.max = Monitor::size();

    auto error = []() {
        std::cerr << "Window: Failed to create new window" << std::endl;
        Window::close();
        throw std::runtime_error("Window: Failed to create Window");
    };

    std::clog << "Window: Creating new window '" << title << "'" << std::endl;

    uint32_t attrib = to_sdl_attrib(attribs);
    data._M_title = title;

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, data._M_X11_compositing);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, cast(int, data._M_is_transparent_framebuffer));

    sdl_window = SDL_CreateWindow(data._M_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cast(int, width),
                                  cast(int, height), SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | attrib);


    if (sdl_window == nullptr)
        error();

    data._M_GL_context = SDL_GL_CreateContext(sdl_window);
    if (data._M_GL_context == nullptr)
        error();

    SDL_GL_MakeCurrent(sdl_window, data._M_GL_context);

#ifndef __ANDROID__
    if (glewInit() != GLEW_OK)
        error();
#endif

    glViewport(0, 0, width, height);
    data._M_size = {width, height};
    data._M_ration = {width, height};


    Window::size_limits(data._M_limits);

    // Init shaders
    Engine::ShaderSystem::init();
    data._M_is_inited = true;

    return window;
}


static const Window& Vulkan_window_init(float width, float height, const std::string& title,
                                        const std::list<WindowAttrib>& attribs)
{
    throw not_implemented;
    return window;
}

static const Window& (*init_window[])(float, float, const std::string&,
                                      const std::list<WindowAttrib>&) = {OpenGL_window_init, Vulkan_window_init};


const Window& Window::init(float width, float height, const std::string& title, uint16_t attrib)
{
    if (data._M_is_inited)
        return window;

    if (!Engine::is_inited())
        throw std::runtime_error("Window: Init Engine first");

    init_window[static_cast<unsigned int>(api)](width, height, title, parse_win_attibutes(attrib));
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return window;
}

const Window& Window::init(const Size2D& size, const std::string& title, uint16_t attrib)
{
    return init(size.x, size.y, title, attrib);
}


//          CLOSING WINDOW

static const Window& OpenGL_window_close()
{
    check_init(window);

    std::clog << "Engine::Window: Window '" << data._M_title << "' closed" << std::endl;

    if (data._M_GL_context)
        SDL_GL_DeleteContext(data._M_GL_context);

    if (data._M_window)
        SDL_DestroyWindow(data._M_window);

    // Reset window data
    data = WindowData();
    return window;
}

static const Window& Vulkan_window_close()
{
    throw not_implemented;
    return window;
}

static const Window& (*window_close[])() = {OpenGL_window_close, Vulkan_window_close};


const Window& Window::close()
{
    check_init(window);
    free_icon_surface();
    std::clog << "Closing window" << std::endl;
    return window_close[cast(unsigned int, api)]();
}


bool Window::is_open()
{
    return data._M_is_inited;
}


//          SWAPPING BUFFERS

static const Window& OpenGL_win_swap_buffers()
{
    SDL_GL_SwapWindow(data._M_window);
    return window;
}


static const Window& Vulkan_win_swap_buffers()
{
    throw not_implemented;
    return window;
}

static const Window& (*win_swap_buffers[])() = {OpenGL_win_swap_buffers, Vulkan_win_swap_buffers};


const Window& Window::swap_buffers()
{
    check_init(window);
    return win_swap_buffers[cast(unsigned int, api)]();
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


//          WINDOW EVENT SYSTEM IMPLEMENTATION


static void window_event(SDL_WindowEvent& event)
{
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
            break;
        }

        case SDL_WINDOWEVENT_ENTER:
        {
            break;
        }

        case SDL_WINDOWEVENT_LEAVE:
        {
            data._M_mouse_offset = data._M_mouse_position = {0, 0};
            break;
        }

        case SDL_WINDOWEVENT_CLOSE:
            window.close();
            break;
    }
}

static void mouse_motion_event(SDL_MouseMotionEvent& event)
{
    data._M_mouse_position = {event.x, event.y};
    data._M_mouse_offset = {event.xrel, event.yrel};
}

static void mouse_button_event(SDL_MouseButtonEvent& event)
{
    Key key = to_key(event.button);
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        data.keys._M_keys[key] = KeyStatus::JUST_PRESSED;
        data.keys._M_last_mouse_key = key;
    }
    else
    {
        data.keys._M_keys[key] = KeyStatus::JUST_RELEASED;
        data.keys._M_last_mouse_released = key;
    }

    data.keys._M_last_evented_keys.push_back(key);
}


static void mouse_wheel_event(SDL_MouseWheelEvent& event)
{
    data._M_mouse_offset = {event.preciseX, event.preciseY};
}

static void paths_event(SDL_DropEvent& event)
{
    if (event.file)
    {
        data._M_dropped_paths.push_back(event.file);
        SDL_free(event.file);
    }
}

static void key_pressed_event(SDL_KeyboardEvent& event)
{
    Key key = to_key(event.keysym.scancode);
    KeyStatus& status = data.keys._M_keys[key];

    if (event.repeat)
    {
        status = KeyStatus::REPEAT;
    }
    else if (event.type == SDL_KEYDOWN)
    {
        status = KeyStatus::JUST_PRESSED;
        data.keys._M_last_key = key;
        data.keys._M_last_evented_keys.push_back(key);
    }
    else
    {
        status = KeyStatus::JUST_RELEASED;
        data.keys._M_last_released = key;
        data.keys._M_last_evented_keys.push_back(key);
    }
}

static void check_event()
{

    switch (data.event.type)
    {

        case SDL_QUIT:
            window.close();
            break;

        case SDL_WINDOWEVENT:
            window_event(data.event.window);
            break;

        case SDL_MOUSEMOTION:
        {
            mouse_motion_event(data.event.motion);
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            mouse_button_event(data.event.button);
            break;
        }
        case SDL_MOUSEWHEEL:
        {
            mouse_wheel_event(data.event.wheel);
            break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            key_pressed_event(data.event.key);
            break;
        }
        case SDL_DROPFILE:
        case SDL_DROPCOMPLETE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
            paths_event(data.event.drop);
            break;
        default:
            break;
    }

    for (auto& func : data._M_event_callback) func(static_cast<void*>(&data.event));
}


const Window& Window::Event::poll_events()
{
    check_init(window);
    event_preprocessing();
    for (; SDL_PollEvent(&data.event);) check_event();
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return window;
}

const Window& Window::Event::wait_for_events()
{
    check_init(window);
    event_preprocessing();
    if (SDL_WaitEvent(&data.event))
        check_event();
    data._M_flags = SDL_GetWindowFlags(sdl_window);
    return window;
}


std::size_t Window::Event::diff_time()
{
    return data._M_diff_time;
}

std::size_t Window::Event::time()
{
    return data._M_time;
}

KeyStatus Window::Event::get_key_status(const Key& key)
{
    return data.keys._M_keys[cast(unsigned int, key)];
}

bool Window::Event::pressed(const Key& key)
{
    return data.keys._M_keys[cast(unsigned int, key)] != KeyStatus::RELEASED &&
           data.keys._M_keys[cast(unsigned int, key)] != KeyStatus::JUST_RELEASED;
}

//      KEYBOARD EVENTS

const Key Window::Event::Keyboard::just_pressed()
{
    return data.keys._M_keys[data.keys._M_last_key] == KeyStatus::JUST_PRESSED ? data.keys._M_last_key : KEY_UNKNOWN;
}

unsigned int Window::Event::Keyboard::last_symbol(bool reset)
{
    unsigned int res = data._M_last_symbol;
    if (reset)
        data._M_last_symbol = 0;
    return res;
}

const Key Window::Event::Keyboard::last_pressed()
{
    return to_key(data.keys._M_last_key + 1);
}

const Key Window::Event::Keyboard::just_released()
{
    return data.keys._M_keys[data.keys._M_last_released] == KeyStatus::JUST_RELEASED ? data.keys._M_last_released
                                                                                     : KEY_UNKNOWN;
}


//      MOUSE EVENTS

const Point2D& Window::Event::Mouse::position()
{
    return data._M_mouse_position;
}
const Window& Window::Event::Mouse::position(const Point2D& position)
{
    check_init(window);
    SDL_WarpMouseInWindow(sdl_window, cast(int, position.x), cast(int, position.y));
    return window;
}

const Offset2D& Window::Event::Mouse::offset()
{
    return data._M_mouse_offset;
}

const Offset2D& Window::Event::Mouse::scroll_offset()
{
    return data._M_scroll_offset;
}

const Key Window::Event::Mouse::just_pressed()
{
    return data.keys._M_keys[data.keys._M_last_mouse_key] == KeyStatus::JUST_PRESSED
                   ? to_key(data.keys._M_last_mouse_key - 1)
                   : KEY_UNKNOWN;
}

const Key Window::Event::Mouse::last_pressed()
{
    return to_key(data.keys._M_last_mouse_key + 1);
}

const Key Window::Event::Mouse::just_released()
{
    return data.keys._M_keys[data.keys._M_last_mouse_released] == KeyStatus::JUST_RELEASED
                   ? to_key(data.keys._M_last_mouse_released - 1)
                   : KEY_UNKNOWN;
}

//          VSYNC

static const Window& OpenGL_swap_interval(int value)
{
    SDL_GL_SetSwapInterval(value);
    return window;
}

static const Window& Vulkan_swap_interval(int value)
{
    throw not_implemented;
    return window;
}

const Window& (*win_swap_interval[])(int) = {OpenGL_swap_interval, Vulkan_swap_interval};

bool Window::vsync()
{
    return data._M_swap_interval > 0;
}

const Window& Window::vsync(const bool& value)
{
    return swap_interval(cast(int, value));
}

int Window::swap_interval()
{
    return data._M_swap_interval;
}

const Window& Window::swap_interval(int value)
{
    check_init(window);
    return win_swap_interval[cast(int, api)](value);
}


//          WINDOW TITLE

const std::string& Window::title()
{
    return data._M_title;
}

const Window& Window::title(const std::string& title)
{

    check_init(window);
    data._M_title = title;
    SDL_SetWindowTitle(sdl_window, data._M_title.c_str());
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
const std::vector<std::string>& Window::dropped_paths()
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
    return cast(bool, data._M_flags& SDL_WINDOW_INPUT_FOCUS);
}

Color& Window::background_color()
{
    return data._M_background_color;
}


static const Window& OpenGL_background_color()
{
    glClearColor(cast(GLclampf, data._M_background_color.r), cast(GLclampf, data._M_background_color.g),
                 cast(GLclampf, data._M_background_color.b), cast(GLclampf, data._M_background_color.a));
    return window;
}

static const Window& Vulkan_background_color()
{
    throw not_implemented;
    return window;
}


static const Window& (*set_background_color[])() = {OpenGL_background_color, Vulkan_background_color};

const Window& Window::background_color(const Color& color)
{
    check_init(window);
    data._M_background_color = color;
    return set_background_color[cast(int, api)]();
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
    return cast(bool, data._M_flags& SDL_WINDOW_SHOWN);
}


static const Window& OpenGL_clear_buffer(const BufferType& buffer)
{
    int buffer_type = Engine::OpenGL::get_buffer(buffer);
    if (buffer_type)
        glClear(buffer_type);


    return window;
}

static const Window& Vulkan_clear_buffer(const BufferType& buffer)
{
    throw not_implemented;
    return window;
}

static const Window& (*win_clear_buffer[])(const BufferType&) = {OpenGL_clear_buffer, Vulkan_clear_buffer};


const Window& Window::clear_buffer(const BufferType& buffer)
{
    check_init(window);
    return win_clear_buffer[cast(int, api)](buffer);
}

bool Window::is_iconify()
{
    check_init(false);
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

const Window& Window::icon(const std::string& image)
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
                        std::clog << "Window: Cannot change flag WIN_TRANSPARENT_FRAMEBUFFER after creating window"
                                  << std::endl;
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
        std::clog << "Window: " << e.what() << std::endl;
    }

    return window;
}

bool Window::attribute(const WindowAttrib& attrib)
{
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
        std::clog << e.what() << std::endl;
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
    SDL_ShowCursor((mode == CursorMode::HIDDEN ? SDL_DISABLE : SDL_ENABLE));
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return window;
}

const Window& Window::update_view_port()
{
    check_init(window);
    win_viewport_resize[cast(int, api)]();
    return window;
}

std::vector<void (*)(void*)>& Window::event_callbacks()
{
    return data._M_event_callback;
}

// Window constructors

Window::Window()
{
    data._M_objects++;
}

Window::Window(float width, float height, const std::string& title, uint16_t attrib) : Window()
{
    init(width, height, title, attrib);
}

Window::Window(const Size2D& size, const std::string& title, uint16_t attrib) : Window()
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
