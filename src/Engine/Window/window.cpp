#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Graphics/shader_system.hpp>
#include <Init/init.hpp>
#include <Window/window.hpp>
#include <iostream>
#include <opengl.hpp>

using namespace Engine;


static Window window;
#define win_init_error throw std::runtime_error("Init window first")

#define glfw_window data._M_window
#define check_init(value)                                                                                                   \
    if (!data._M_is_inited)                                                                                                 \
    return value

const EngineAPI& api = Engine::API();
static constexpr Point2D default_point(-1.f);


static const std::unordered_map<WindowAttrib, int> window_attributes = {
        {WindowAttrib::RESIZABLE, GLFW_RESIZABLE},
        {WindowAttrib::VISIBLE, GLFW_VISIBLE},
        {WindowAttrib::DECORATED, GLFW_DECORATED},
        {WindowAttrib::FOCUSED, GLFW_FOCUSED},
        {WindowAttrib::AUTO_ICONIFY, GLFW_AUTO_ICONIFY},
        {WindowAttrib::FLOATING, GLFW_FLOATING},
        {WindowAttrib::MAXIMIZED, GLFW_MAXIMIZED},
        {WindowAttrib::CENTER_CURSOR, GLFW_CENTER_CURSOR},
        {WindowAttrib::TRANSPARENT_FRAMEBUFFER, GLFW_TRANSPARENT_FRAMEBUFFER},
        {WindowAttrib::FOCUS_ON_SHOW, GLFW_FOCUS_ON_SHOW},
        {WindowAttrib::SCALE_TO_MONITOR, GLFW_SCALE_TO_MONITOR}};


static struct WindowData {
    GLFWwindow* _M_window = nullptr;
    bool _M_is_inited = false;
    std::string _M_title;

    SizeLimits2D _M_limits;
    Size2D _M_size = {-1, -1};
    Size2D _M_position;

    int _M_swap_interval = 1;

    std::vector<std::string> _M_dropped_paths;

    Color _M_background_color = Color(0, 0, 0, 1);
    Cursor _M_cursor;

    Image _M_icon;
    AspectRation _M_ration;
    bool _M_enable_ration = false;
    WindowMode _M_mode = WindowMode::NONE;
    std::vector<int> _M_backup;
    CursorMode _M_cursor_mode = CursorMode::NORMAL;

    struct Keys {
        std::vector<KeyStatus> _M_keys;
        int _M_last_key = -1;
        int _M_last_released;

        int _M_last_mouse_key = -1;
        int _M_last_mouse_released;


        Keys() : _M_keys(GLFW_KEY_LAST + 2, RELEASED)
        {
            _M_last_key = 0;
            _M_last_mouse_key = 0;
            _M_last_mouse_released = 0;
            _M_last_released = 0;
        }

    } keys;

    float _M_time = .0f;
    float _M_diff_time = .0f;
    float _M_last_time = .0f;

    unsigned int _M_last_symbol = 0;

    Point2D _M_mouse_position;
    Offset2D _M_mouse_offset;
    Offset2D _M_scroll_offset;

    std::size_t _M_objects = 0;

} data;


//          CALLBACKS


//      RESIZE CALLBACK
static void OpenGL_win_resize()
{
    glViewport(0, 0, cast(GLsizei, data._M_size.x), cast(GLsizei, data._M_size.y));
}

static void Vulkan_win_resize()
{
    throw not_implemented;
}

static void (*win_resize[])() = {OpenGL_win_resize, Vulkan_win_resize};

static void size_callback(GLFWwindow*, int x, int y)
{
    data._M_size = {cast(Size1D, x), cast(Size1D, y)};

    if (!data._M_enable_ration)
        data._M_ration = {x, y};
    win_resize[cast(int, api)]();
}


//      POSITION CALLBACK

static void pos_callback(GLFWwindow*, int x, int y)
{
    data._M_position = {cast(Size1D, x), cast(Size1D, y)};
}

//      DROPPED PATHS CALLBACK

static void dropped_path_callback(GLFWwindow*, int path_count, const char* paths[])
{
    for (int i = 0; i < path_count; i++) data._M_dropped_paths.push_back(paths[i]);
}

//      KEY CALLBACK

static void key_callback(GLFWwindow*, int key, int scancode, int action, int mods)
{
    data.keys._M_keys[key + 1] = action == GLFW_PRESS ? JUST_PRESSED : action == GLFW_REPEAT ? REPEAT : JUST_RELEASED;
    if (action != GLFW_RELEASE)
        data.keys._M_last_key = key + 1;
    else
        data.keys._M_last_released = key + 1;
}

//      CHARACTER CALLBACK

static void character_callback(GLFWwindow*, unsigned int codepoint)
{
    data._M_last_symbol = codepoint;
}

//      CLOSE CALLBACK
static void close_callback(GLFWwindow*)
{
    window.close();
}

//      CURSOR CALLBACK

static void cursor_callback(GLFWwindow*, double x, double y)
{
    Point2D current = {cast(float, x), data._M_size.y - cast(float, y)};
    if (data._M_mouse_position != default_point && current != default_point)
        data._M_mouse_offset = current - data._M_mouse_position;
    data._M_mouse_position = current;
}

//      CURSOR ENTERED CALLBACK
static void cursor_entered_callback(GLFWwindow*, int status)
{
    if (!status)
    {
        data._M_mouse_position = {-1, -1};
        data._M_mouse_offset = {0, 0};
    }
}

//      CURSOR SCROLL OFFSET
static void scroll_callback(GLFWwindow*, double x, double y)
{
    data._M_scroll_offset = {cast(float, x), cast(float, y)};
}

//      MOUSE KEY CALLBACK
static void mouse_buttons_callback(GLFWwindow*, int button, int action, int)
{
    data.keys._M_keys[button + 1] = action == GLFW_PRESS ? JUST_PRESSED : JUST_RELEASED;
    if (action != GLFW_RELEASE)
        data.keys._M_last_mouse_key = button + 1;
    else
        data.keys._M_last_mouse_released = button + 1;
}


static void event_preprocessing()
{
    // Keyboard
    if (data.keys._M_keys[data.keys._M_last_key] == JUST_PRESSED)
        data.keys._M_keys[data.keys._M_last_key] = PRESSED;
    if (data.keys._M_keys[data.keys._M_last_released] == JUST_RELEASED)
        data.keys._M_keys[data.keys._M_last_released] = RELEASED;

    // Mouse
    if (data.keys._M_keys[data.keys._M_last_mouse_key] == JUST_PRESSED)
        data.keys._M_keys[data.keys._M_last_mouse_key] = PRESSED;
    if (data.keys._M_keys[data.keys._M_last_mouse_released] == JUST_RELEASED)
        data.keys._M_keys[data.keys._M_last_mouse_released] = RELEASED;

    data._M_scroll_offset = {0, 0};
    data._M_mouse_offset = {0, 0};

    data._M_time = glfwGetTime();
    data._M_diff_time = data._M_time - data._M_last_time;
    data._M_last_time = data._M_time;
}


//          WINDOW INITIALIZATION

static const Window& OpenGL_window_init(float width, float height, const std::string& title, bool rezisable)
{
    data._M_limits.max = Monitor::size();

    auto error = []() {
        std::cerr << "Window: Failed to create new window" << std::endl;
        throw std::runtime_error("Window: Failed to create Window");
    };

    std::clog << "Window: Creating new window '" << title << "'" << std::endl;

    data._M_title = title;
    glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(rezisable));

    data._M_window = glfwCreateWindow(cast(int, width), cast(int, height), data._M_title.c_str(), nullptr, nullptr);

    if (data._M_window == nullptr)
        error();

    glfwMakeContextCurrent(data._M_window);

    if (glewInit() != GLEW_OK)
        error();
    glViewport(0, 0, width, height);
    data._M_size = {width, height};
    data._M_ration = {width, height};

    glfwSetWindowSizeLimits(glfw_window, cast(int, data._M_limits.min.x), cast(int, data._M_limits.min.y),
                            cast(int, data._M_limits.max.x), cast(int, data._M_limits.max.y));

    // Callbacks
    glfwSetWindowSizeCallback(glfw_window, size_callback);
    glfwSetWindowPosCallback(glfw_window, pos_callback);
    glfwSetDropCallback(glfw_window, dropped_path_callback);
    glfwSetKeyCallback(glfw_window, key_callback);
    glfwSetWindowCloseCallback(glfw_window, close_callback);
    glfwSetCharCallback(glfw_window, character_callback);
    glfwSetCursorPosCallback(glfw_window, cursor_callback);
    glfwSetCursorEnterCallback(glfw_window, cursor_entered_callback);
    glfwSetScrollCallback(glfw_window, scroll_callback);
    glfwSetMouseButtonCallback(glfw_window, mouse_buttons_callback);

    // Init shaders
    Engine::ShaderSystem::init();
    data._M_is_inited = true;

    return window;
}


static const Window& Vulkan_window_init(float width, float height, const std::string& title, bool rezisable)
{
    throw not_implemented;
    return window;
}

static const Window& (*init_window[])(float, float, const std::string&, bool) = {OpenGL_window_init, Vulkan_window_init};


const Window& Window::init(float width, float height, const std::string& title, bool rezisable)
{
    if (data._M_is_inited)
        return window;

    if (!Engine::is_inited())
        throw std::runtime_error("Window: Init Engine first");
    return init_window[static_cast<unsigned int>(api)](width, height, title, rezisable);
}

const Window& Window::init(const Size2D& size, const std::string& title, bool rezisable)
{
    return init(size.x, size.y, title, rezisable);
}


//          CLOSING WINDOW

static const Window& OpenGL_window_close()
{
    check_init(window);

    std::clog << "Engine::Window: Window '" << data._M_title << "' closed" << std::endl;
    glfwDestroyWindow(glfw_window);

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
    std::clog << "Closing window" << std::endl;
    return window_close[cast(unsigned int, api)]();
}


bool Window::is_open()
{
    return data._M_is_inited && !glfwWindowShouldClose(glfw_window);
}


//          SWAPPING BUFFERS

static const Window& OpenGL_win_swap_buffers()
{
    glfwSwapBuffers(glfw_window);
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
    glfwSetWindowSize(glfw_window, cast(int, width), cast(int, data._M_size.y));
    return window;
}

Size1D Window::height()
{
    return data._M_size.y;
}

const Window& Window::height(const Size1D& height)
{
    check_init(window);
    glfwSetWindowSize(glfw_window, cast(int, data._M_size.x), cast(int, height));
    return window;
}

const Size2D& Window::size()
{
    return data._M_size;
}

const Window& Window::size(const Size2D& size)
{
    check_init(window);
    glfwSetWindowSize(glfw_window, cast(int, size.x), cast(int, size.y));
    return window;
}


//          WINDOW EVENT SYSTEM IMPLEMENTATION

const Window& Window::Event::poll_events()
{
    check_init(window);
    event_preprocessing();
    glfwPollEvents();
    return window;
}

const Window& Window::Event::wait_for_events()
{
    check_init(window);
    event_preprocessing();
    glfwWaitEvents();
    return window;
}


float Window::Event::diff_time()
{
    return data._M_diff_time;
}

float Window::Event::time()
{
    return data._M_time;
}

KeyStatus Window::Event::get_key_status(const Key& key)
{
    return data.keys._M_keys[to_glfw_key(key) + 1];
}

bool Window::Event::pressed(const Key& key)
{
    int glfw_key = to_glfw_key(key) + 1;
    return data.keys._M_keys[glfw_key] != RELEASED && data.keys._M_keys[glfw_key] != JUST_RELEASED;
}

//      KEYBOARD EVENTS

const Key Window::Event::Keyboard::just_pressed()
{
    return data.keys._M_keys[data.keys._M_last_key] == JUST_PRESSED ? to_key(data.keys._M_last_key - 1) : KEY_UNKNOWN;
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
    return data.keys._M_keys[data.keys._M_last_released] == JUST_RELEASED ? to_key(data.keys._M_last_released - 1)
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
    glfwSetCursorPos(glfw_window, cast(double, position.x), cast(double, position.y));
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
    return data.keys._M_keys[data.keys._M_last_mouse_key] == JUST_PRESSED ? to_key(data.keys._M_last_mouse_key - 1)
                                                                          : KEY_UNKNOWN;
}

const Key Window::Event::Mouse::last_pressed()
{
    return to_key(data.keys._M_last_mouse_key + 1);
}

const Key Window::Event::Mouse::just_released()
{
    return data.keys._M_keys[data.keys._M_last_mouse_released] == JUST_RELEASED
                   ? to_key(data.keys._M_last_mouse_released - 1)
                   : KEY_UNKNOWN;
}

//          VSYNC

static const Window& OpenGL_swap_interval(int value)
{
    glfwSwapInterval(value);
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
    glfwSetWindowTitle(glfw_window, data._M_title.c_str());

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
        glfwSetWindowPos(glfw_window, cast(int, position.x), cast(int, position.y));
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
    return data._M_is_inited ? glfwGetWindowAttrib(glfw_window, GLFW_RESIZABLE) == 1 : false;
}

const Window& Window::rezisable(bool value)
{
    check_init(window);
    glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, cast(int, value));
    return window;
}

//  FOCUS WINDOW

const Window& Window::focus()
{
    check_init(window);

    glfwFocusWindow(glfw_window);
    return window;
}

bool Window::focused()
{
    check_init(false);
    return cast(bool, glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED));
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
    glfwShowWindow(glfw_window);
    return window;
}

const Window& Window::hide()
{
    check_init(window);
    glfwHideWindow(glfw_window);
    return window;
}

bool Window::is_visible()
{
    check_init(false);
    return cast(bool, glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE));
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
    return cast(bool, glfwGetWindowAttrib(glfw_window, GLFW_ICONIFIED));
}

const Window& Window::iconify()
{
    check_init(window);
    glfwIconifyWindow(glfw_window);
    return window;
}

bool Window::is_restored()
{
    check_init(false);
    return !cast(bool, glfwGetWindowAttrib(glfw_window, GLFW_ICONIFIED));
}

const Window& Window::restore()
{
    check_init(window);
    glfwRestoreWindow(glfw_window);
    return window;
}


const Window& Window::opacity(float value)
{
    check_init(window);
    glfwSetWindowOpacity(glfw_window, value);
    return window;
}

float Window::opacity()
{
    check_init(0.f);
    return glfwGetWindowOpacity(glfw_window);
}

bool Window::center_cursor()
{
    check_init(false);
    return cast(int, glfwGetWindowAttrib(glfw_window, GLFW_CENTER_CURSOR));
}

const Window& Window::center_cursor(bool value)
{
    check_init(window);
    glfwSetWindowAttrib(glfw_window, GLFW_CENTER_CURSOR, cast(int, value));
    return window;
}

const Window& Window::size_limits(const SizeLimits2D& limits)
{
    check_init(window);
    data._M_limits = limits;
    glfwSetWindowSizeLimits(glfw_window, cast(int, limits.min.x), cast(int, limits.min.y), cast(int, limits.max.x),
                            cast(int, limits.max.y));
    return window;
}

const SizeLimits2D& size_limits()
{
    return data._M_limits;
}

const Window& Window::cursor(const Cursor& cursor)
{
    check_init(window);
    glfwSetCursor(glfw_window, cast(GLFWcursor*, cursor.glfw_cursor()));
    data._M_cursor = cursor;
    return window;
}

const Cursor& Window::cursor()
{
    return data._M_cursor;
}

const Window& Window::icon(const Image& image)
{
    check_init(window);
    data._M_icon = image;
    data._M_icon.add_alpha_channel();
    if (!data._M_icon.empty())
        glfwSetWindowIcon(glfw_window, 1, cast(GLFWimage*, data._M_icon.glfw_image()));
    return window;
}

const Window& Window::icon(const std::string& image)
{
    check_init(window);
    data._M_icon.load(image).add_alpha_channel();
    if (!data._M_icon.empty())
        glfwSetWindowIcon(glfw_window, 1, cast(GLFWimage*, data._M_icon.glfw_image()));
    return window;
}

const Image& Window::icon()
{
    return data._M_icon;
}

const Window& Window::aspect_ration(const AspectRation& ration)
{
    check_init(window);
    if ((data._M_enable_ration = (ration.x * ration.y > 0)))
    {
        glfwSetWindowAspectRatio(glfw_window, ration.x, ration.y);
        data._M_ration = ration;
    }
    else
        glfwSetWindowAspectRatio(glfw_window, GLFW_DONT_CARE, GLFW_DONT_CARE);

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
        glfwSetWindowAttrib(glfw_window, window_attributes.at(attrib), cast(int, value));
    }
    catch (const std::exception& e)
    {
        std::clog << e.what() << std::endl;
    }
    return window;
}

bool Window::attribute(const WindowAttrib& attrib)
{
    check_init(false);
    try
    {
        return cast(bool, glfwGetWindowAttrib(glfw_window, window_attributes.at(attrib)));
    }
    catch (const std::exception& e)
    {
        std::clog << e.what() << std::endl;
        return false;
    }
}

const Window& Window::mode(const WindowMode& mode, const Size2D& _size)
{
    check_init(window);
    if (mode == data._M_mode)
        return window;
    // [width, height, x, y]

    if (mode == WindowMode::NONE)
    {
        glfwSetWindowMonitor(glfw_window, nullptr, data._M_backup[0], data._M_backup[1], data._M_backup[2],
                             data._M_backup[3], Monitor::refresh_rate());
        data._M_backup.clear();
    }
    else
    {
        if (data._M_mode == WindowMode::NONE)
        {
            data._M_backup.clear();
            auto pos = position();
            auto current_size = window.size();
            data._M_backup.push_back(pos.x);
            data._M_backup.push_back(pos.y);
            data._M_backup.push_back(current_size.x);
            data._M_backup.push_back(current_size.y);
        }

        int w = 0, h = 0;
        w = (_size.x < 0 || mode == WindowMode::FULLSCREEN) ? Monitor::width() : _size.x;
        h = (_size.y < 0 || mode == WindowMode::FULLSCREEN) ? Monitor::height() : _size.y;
        std::clog << "Window: Set size " << glm::vec<2, int, glm::defaultp>(w, h) << std::endl;
        glfwSetWindowMonitor(glfw_window,
                             mode == WindowMode::FULLSCREEN ? reinterpret_cast<GLFWmonitor*>(Monitor::monitor()) : nullptr,
                             0, 0, w, h, Monitor::refresh_rate());
    }
    data._M_mode = mode;
    return window;
}

const WindowMode& Window::mode()
{
    return data._M_mode;
}


static const std::unordered_map<CursorMode, int> cursor_modes = {{CursorMode::NORMAL, GLFW_CURSOR_NORMAL},
                                                                 {CursorMode::DISABLED, GLFW_CURSOR_DISABLED},
                                                                 {CursorMode::HIDDEN, GLFW_CURSOR_HIDDEN}};

const Window& Window::cursor_mode(const CursorMode& mode)
{
    check_init(window);
    try
    {
        glfwSetInputMode(glfw_window, GLFW_CURSOR, cursor_modes.at(mode));
        data._M_cursor_mode = mode;
    }
    catch (const std::exception& e)
    {
        std::clog << e.what() << std::endl;
    }
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
    win_resize[cast(int, api)]();
    return window;
}

// Window constructors

Window::Window()
{
    data._M_objects++;
}

Window::Window(float width, float height, const std::string& title, bool rezisable) : Window()
{
    init(width, height, title, rezisable);
}

Window::Window(const Size2D& size, const std::string& title, bool rezisable) : Window()
{
    init(size.x, size.y, title, rezisable);
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
