#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Init/init.hpp>
#include <Window/window.hpp>
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>


#define glfw_window static_cast<GLFWwindow*>(parameters._M_window)

#define CHECK(type)                                                                                                                   \
    if (parameters._M_is_closed == true)                                                                                              \
    return static_cast<type>(-1)

#define ENGINE_WINDOW                                                                                                                 \
    auto engine_window = find_window(window);                                                                                         \
    if (engine_window == nullptr)                                                                                                     \
    {                                                                                                                                 \
        std::cerr << "Engine::Window size callback: Filed to found window" << std::endl;                                              \
        return;                                                                                                                       \
    }

#define IMPLEMENT(struct_name, first_param, second_param, type)                                                                       \
    struct_name::struct_name() = default;                                                                                             \
    struct_name::struct_name(const type& first_param, const type& second_param)                                                       \
        : first_param(first_param), second_param(second_param)                                                                        \
    {}                                                                                                                                \
    struct_name& struct_name::operator=(const struct_name&) = default;                                                                \
    struct_name::struct_name(const std::initializer_list<type>& list)                                                                 \
    {                                                                                                                                 \
        auto begin = list.begin();                                                                                                    \
        if (begin == list.end())                                                                                                      \
            return;                                                                                                                   \
        first_param = (*begin++);                                                                                                     \
        if (begin == list.end())                                                                                                      \
            return;                                                                                                                   \
        second_param = (*begin);                                                                                                      \
    }                                                                                                                                 \
    struct_name::struct_name(const struct_name&) = default;


static Engine::Window* current_window = nullptr;
static std::list<Engine::Window*> windows;
using WEvent = Engine::Window::WindowEvent;


namespace Monitor
{

    GLFWmonitor* monitor;
    GLFWvidmode videomode;
    bool monitorIsinited;
    static void initMonitor();
    static void updateInfo();

    static void monitor_callback(GLFWmonitor* _monitor, int value)
    {

        if (value)
        {
            if (_monitor != monitor)
                initMonitor();
            else
                updateInfo();
        }
        else
        {
            std::cerr << "MONITOR: Monitor dissconected" << std::endl;
            monitorIsinited = false;
        }
    }

    static void initMonitor()
    {
        std::clog << "MONITOR: Reading Monitor data" << std::endl;
        monitor = glfwGetPrimaryMonitor();
        if (monitor == nullptr)
        {
            std::cerr << "MONITOR: Failed to read Monitor data" << std::endl;
            return;
        }

        updateInfo();
        glfwSetMonitorCallback(monitor_callback);
        monitorIsinited = true;
    }

    static void updateInfo()
    {
        auto _vidmode = glfwGetVideoMode(monitor);
        if (_vidmode == nullptr)
        {
            monitor = nullptr;
            std::clog << "MONITOR: Failed to read Monitor data" << std::endl;
            return;
        }

        videomode = *_vidmode;
    }

    int getRedBits()
    {
        return monitorIsinited ? videomode.redBits : -1;
    }

    int getGreenBits()
    {
        return monitorIsinited ? videomode.greenBits : -1;
    }

    int getBlueBits()
    {
        return monitorIsinited ? videomode.blueBits : -1;
    }

    int getWidth()
    {
        return monitorIsinited ? videomode.width : -1;
    }

    int getHeight()
    {
        return monitorIsinited ? videomode.height : -1;
    }

    int getRefreshRate()
    {
        return monitorIsinited ? videomode.refreshRate : -1;
    }

}// namespace Monitor

static class MonitorController
{
public:
    MonitorController()
    {
        Monitor::initMonitor();
    }
} monitor;


namespace Engine
{
    //                  SOME STRUCT IMPLEMENTING
    IMPLEMENT(AspectRation, numer, denom, int);
    IMPLEMENT(SizeLimits, min_size, max_size, WindowSize);

    //                  OPERATOR OVERLOADING


    std::ostream& operator<<(std::ostream& stream, const Engine::WindowMode& mode)
    {
        if (mode == Engine::NONE)
            return stream << "NONE";
        if (mode == Engine::FULLSCREEN)
            return stream << "FULLSCREEN";
        else
            return stream << "WIN_FULLSCREEN";
    }

    std::ostream& print(const glm::vec2& vector, std::ostream& stream)
    {
        return stream << "{" << vector.x << " : " << vector.y << "}";
    }

    std::ostream& operator<<(std::ostream& stream, const SizeLimits& limits)
    {
        return print(limits.max_size, print(limits.min_size, stream << "{") << " : ") << "}";
    }

    std::ostream& operator<<(std::ostream& stream, const AspectRation& ar)
    {
        return stream << "{" << ar.numer << " : " << ar.denom << "}";
    }


    //                   CALLBACKS
    WindowParameters& get_parameters(Window* window)
    {
        return (*window).parameters;
    }

    Window* find_window(GLFWwindow* window)
    {
        for (auto address : windows)
        {
            if (get_parameters(address)._M_window == static_cast<void*>(window))
                return address;
        }
        return nullptr;
    }

    void window_size_callback(GLFWwindow* window, int width, int height)
    {
        ENGINE_WINDOW
        engine_window->make_current();
        glViewport(0, 0, width, height);
    }

    void dropped_path_callback(GLFWwindow* window, int count, const char* paths[])
    {
        ENGINE_WINDOW
        Engine::WindowParameters& parameters = get_parameters(engine_window);
        for (int i = 0; i < count; i++) parameters._M_dropped_paths.push_back(paths[i]);
    }

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        ENGINE_WINDOW
        Engine::WindowParameters& parameters = get_parameters(engine_window);
        parameters.keys._M_keys[key + 1] = action == GLFW_PRESS ? JUST_PRESSED : action == GLFW_REPEAT ? REPEAT : JUST_RELEASED;
        if (action != GLFW_RELEASE)
            parameters.keys._M_last_key = key + 1;
        else
            parameters.keys._M_last_released = key + 1;
    }

    void close_callback(GLFWwindow* window)
    {
        ENGINE_WINDOW

        engine_window->destroy();
    }

    void character_callback(GLFWwindow* window, unsigned int codepoint)
    {
        ENGINE_WINDOW
        Engine::WindowParameters& parameters = get_parameters(engine_window);
        parameters._M_last_symbol = codepoint;
    }

    void cursor_callback(GLFWwindow* window, double x, double y)
    {
        ENGINE_WINDOW
        Engine::WindowParameters& parameters = get_parameters(engine_window);
        if (!Engine::float_equal(parameters._M_mouse_position.x, -1.0f, 1) &&
            !Engine::float_equal(parameters._M_mouse_position.y, -1.0f, 1))
        {

            parameters._M_offset.x = static_cast<int>(x) - parameters._M_mouse_position.x;
            parameters._M_offset.y = static_cast<int>(y) - parameters._M_mouse_position.y;
        }
        parameters._M_mouse_position.x = static_cast<int>(x);
        parameters._M_mouse_position.y = static_cast<int>(y);
    }

    void cursor_entered_callback(GLFWwindow* window, int value)
    {
        if (value == 0)
        {
            ENGINE_WINDOW
            Engine::WindowParameters& parameters = get_parameters(engine_window);
            parameters._M_mouse_position = {-1, -1};
        }
    }

    void scroll_callback(GLFWwindow* window, double x, double y)
    {
        ENGINE_WINDOW
        Engine::WindowParameters& parameters = get_parameters(engine_window);
        parameters._M_scroll_offset.x = static_cast<int>(x);
        parameters._M_scroll_offset.y = static_cast<int>(y);
    }

    void mouse_buttons_callback(GLFWwindow* window, int button, int action, int none)
    {

        ENGINE_WINDOW
        Engine::WindowParameters& parameters = get_parameters(engine_window);
        parameters.keys._M_keys[button + 1] = action == GLFW_PRESS ? JUST_PRESSED : JUST_RELEASED;
        if (action != GLFW_RELEASE)
            parameters.keys._M_last_mouse_key = button + 1;
        else
            parameters.keys._M_last_mouse_released = button + 1;
    }


    //          EVENT SYSTEM IMPLEMENTATION

    //                    KEYBOARD
    WEvent::Keyboard::Keyboard(Window* window) : window(window)
    {}

    KeyStatus WEvent::get_key_status(const Key& key)
    {
        int glfw_key = to_glfw_key(key);
        return window->parameters.keys._M_keys[glfw_key + 1];
    }

    const Key WEvent::Keyboard::just_pressed()
    {
        auto& keys = window->parameters.keys;
        return keys._M_keys[keys._M_last_key] == JUST_PRESSED ? to_key(keys._M_last_key - 1) : KEY_UNKNOWN;
    }

    const std::string& WEvent::Keyboard::last_symbol()
    {
        static std::string string_result;
        char result[] = {0, 0, 0, 0, 0};
        unsigned int& cp = window->parameters._M_last_symbol;
        if (cp <= 0x7F)
        {
            result[0] = cp;
        }
        else if (cp <= 0x7FF)
        {
            result[0] = (cp >> 6) + 192;
            result[1] = (cp & 63) + 128;
        }
        else if (0xd800 <= cp && cp <= 0xdfff)
        {
        }//invalid block of utf8
        else if (cp <= 0xFFFF)
        {
            result[0] = (cp >> 12) + 224;
            result[1] = ((cp >> 6) & 63) + 128;
            result[2] = (cp & 63) + 128;
        }
        else if (cp <= 0x10FFFF)
        {
            result[0] = (cp >> 18) + 240;
            result[1] = ((cp >> 12) & 63) + 128;
            result[2] = ((cp >> 6) & 63) + 128;
            result[3] = (cp & 63) + 128;
        }
        string_result = result;
        cp = 0;
        return string_result;
    }

    const Key WEvent::Keyboard::last_pressed()
    {
        return to_key(window->parameters.keys._M_last_key + 1);
    }

    bool WEvent::pressed(const Key& key)
    {
        int glfw_key = to_glfw_key(key) + 1;
        return window->parameters.keys._M_keys[glfw_key] != RELEASED && window->parameters.keys._M_keys[glfw_key] != JUST_RELEASED;
    }
    const Key WEvent::Keyboard::just_released()
    {
        auto& keys = window->parameters.keys;
        return keys._M_keys[keys._M_last_released] == JUST_RELEASED ? to_key(keys._M_last_released - 1) : KEY_UNKNOWN;
    }


    //                  MOUSE
    WEvent::Mouse::Mouse(Window* window) : window(window)
    {}

    const Position2D& WEvent::Mouse::position()
    {
        return window->parameters._M_mouse_position;
    }

    Window& WEvent::Mouse::position(const Position2D& pos)
    {
        glfwSetCursorPos(static_cast<GLFWwindow*>(window->parameters._M_window), static_cast<double>(pos.x),
                         static_cast<double>(pos.y));
        window->parameters._M_mouse_position = pos;
        return *window;
    }

    const Offset& WEvent::Mouse::offset()
    {
        return window->parameters._M_offset;
    }

    const Offset& WEvent::Mouse::scroll_offset()
    {
        return window->parameters._M_scroll_offset;
    }

    Window& WEvent::Mouse::cursor_status(const CursorStatus& status)
    {
        glfwSetInputMode(static_cast<GLFWwindow*>(window->parameters._M_window), GLFW_CURSOR,
                         ((status == NORMAL)     ? GLFW_CURSOR_NORMAL
                          : (status == DISABLED) ? GLFW_CURSOR_DISABLED
                                                 : GLFW_CURSOR_HIDDEN));
        window->parameters._M_cursor_status = status;
        return *window;
    }

    CursorStatus& WEvent::Mouse::cursor_status()
    {
        return window->parameters._M_cursor_status;
    }

    const Key WEvent::Mouse::just_pressed()
    {
        auto& keys = window->parameters.keys;
        return keys._M_keys[keys._M_last_mouse_key] == JUST_PRESSED ? to_key(keys._M_last_mouse_key - 1) : KEY_UNKNOWN;
    }

    const Key WEvent::Mouse::last_pressed()
    {
        return to_key(window->parameters.keys._M_last_mouse_key + 1);
    }

    const Key WEvent::Mouse::just_released()
    {
        auto& keys = window->parameters.keys;
        return keys._M_keys[keys._M_last_mouse_released] == JUST_RELEASED ? to_key(keys._M_last_mouse_released - 1) : KEY_UNKNOWN;
    }


    //                  EVENT
    WEvent::WindowEvent(Window* window) : keyboard(window), mouse(window)
    {
        this->window = window;
    }

    void WEvent::poll_events()
    {
        for (auto& w : windows) w->event_preprocessing();
        glfwPollEvents();
    }

    float WEvent::diff_time()
    {
        return window->parameters._M_diff_time;
    }

    //              WINDOW SYSTEM IMPLEMENTATION


    void Window::event_preprocessing()
    {
        // Keyboard
        if (parameters.keys._M_keys[parameters.keys._M_last_key] == JUST_PRESSED)
            parameters.keys._M_keys[parameters.keys._M_last_key] = PRESSED;
        if (parameters.keys._M_keys[parameters.keys._M_last_released] == JUST_RELEASED)
            parameters.keys._M_keys[parameters.keys._M_last_released] = RELEASED;

        // Mouse
        if (parameters.keys._M_keys[parameters.keys._M_last_mouse_key] == JUST_PRESSED)
            parameters.keys._M_keys[parameters.keys._M_last_mouse_key] = PRESSED;
        if (parameters.keys._M_keys[parameters.keys._M_last_mouse_released] == JUST_RELEASED)
            parameters.keys._M_keys[parameters.keys._M_last_mouse_released] = RELEASED;
        parameters._M_scroll_offset.x = 0;
        parameters._M_scroll_offset.y = 0;
        parameters._M_offset.x = 0;
        parameters._M_offset.y = 0;

        static float last_time = 0;
        float time = glfwGetTime();
        parameters._M_diff_time = time - last_time;
        last_time = time;
    }

    WindowParameters::WindowParameters(const std::string& name) : _M_name(name)
    {
        std::clog << "Window: Creating keys array for '" << name << "' window" << std::endl;
        keys._M_keys = new KeyStatus[GLFW_KEY_LAST + 2];
        std::fill(keys._M_keys, keys._M_keys + GLFW_KEY_LAST + 2, RELEASED);
        _M_is_closed = false;
        _M_dropped_paths = {};
        _M_limits.max_size = {Monitor::getWidth(), Monitor::getWidth()};
        keys._M_last_key = 0;
        keys._M_last_mouse_key = 0;
        keys._M_last_mouse_released = 0;
        keys._M_last_released = 0;
    }

    WindowParameters::WindowParameters(const WindowParameters&) = default;
    WindowParameters::~WindowParameters()
    {
        std::clog << "Window: Deleting keys array for '" << _M_name << "' window" << std::endl;
        delete[] keys._M_keys;
    }


    //              WINDOW CLASS
    Window::Window(int width, int height, std::string name, bool rezisable) : parameters(name), event(this)
    {
        auto error = []() {
            std::cerr << "Window: Failed to create new window" << std::endl;
            throw std::runtime_error("Window: Failed to create Window");
        };

        std::clog << "Window: Creating new window '" << name << "',\taddress of window: " << this << std::endl;
        glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(rezisable));
        parameters._M_window = static_cast<void*>(glfwCreateWindow(width, height, parameters._M_name.c_str(), nullptr, nullptr));

        if (parameters._M_window == nullptr)
        {
            error();
        }

        glfwMakeContextCurrent(glfw_window);

        if (glewInit() != GLEW_OK)
            error();
        glViewport(0, 0, width, height);
        glfwSetWindowSizeLimits(glfw_window, parameters._M_limits.min_size.x, parameters._M_limits.min_size.y,
                                parameters._M_limits.max_size.x, parameters._M_limits.max_size.y);

        // Callbacks
        glfwSetWindowSizeCallback(glfw_window, window_size_callback);
        glfwSetDropCallback(glfw_window, dropped_path_callback);
        glfwSetKeyCallback(glfw_window, key_callback);
        glfwSetWindowCloseCallback(glfw_window, close_callback);
        glfwSetCharCallback(glfw_window, character_callback);
        glfwSetCursorPosCallback(glfw_window, cursor_callback);
        glfwSetCursorEnterCallback(glfw_window, cursor_entered_callback);
        glfwSetScrollCallback(glfw_window, scroll_callback);
        glfwSetMouseButtonCallback(glfw_window, mouse_buttons_callback);
        current_window = this;
        windows.push_back(this);
    }

    Window::~Window()
    {
        destroy();
        auto it = std::find(windows.begin(), windows.end(), this);
        if (it != windows.end())
            windows.erase(it);
    }

    void Window::make_current()
    {
        CHECK(void);
        glfwMakeContextCurrent(glfw_window);
        current_window = this;
    }

    bool Window::is_open() const
    {
        return !glfwWindowShouldClose(glfw_window) && parameters._M_is_closed == false;
    }

    void Window::clear_buffer()
    {
        CHECK(void);
        make_current();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Window::swap_buffers()
    {
        CHECK(void);
        make_current();
        glfwSwapBuffers(glfw_window);
    }

    int Window::width()
    {
        CHECK(int);
        return size().x;
    }

    int Window::height()
    {
        CHECK(int);
        return size().y;
    }

    void Window::clear_dropped_path()
    {
        parameters._M_dropped_paths.clear();
    }

    const std::vector<std::string>& Window::get_dropped_paths()
    {
        return parameters._M_dropped_paths;
    }

    bool Window::rezisable()
    {
        return glfwGetWindowAttrib(glfw_window, GLFW_RESIZABLE) != 0;
    }

    Window& Window::rezisable(bool value)
    {
        glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, value);
        return *this;
    }

    const std::string& Window::title()
    {
        return parameters._M_name;
    }

    Window& Window::title(const std::string& title)
    {
        if (parameters._M_is_closed == true)
            return *this;
        parameters._M_name = title;
        glfwSetWindowTitle(glfw_window, parameters._M_name.c_str());
        return *this;
    }

    WindowSize Window::size()
    {
        if (parameters._M_is_closed == true)
            return {-1, -1};
        int width, height;
        glfwGetWindowSize(glfw_window, &width, &height);
        return {width, height};
    }

    Window& Window::size(const WindowSize& size)
    {
        glfwSetWindowSize(glfw_window, size.x, size.y);
        return *this;
    }

    Window& Window::width(const int& width)
    {
        return size({width, size().y});
    }

    Window& Window::height(const int& height)
    {
        return size({size().x, height});
    }

    const Position2D& Window::position()
    {
        static Position2D pos;
        int x, y;
        glfwGetWindowPos(glfw_window, &x, &y);
        pos.x = static_cast<float>(x);
        pos.y = static_cast<float>(y);
        return pos;
    }

    Window& Window::position(const Position2D& position)
    {
        glfwSetWindowPos(glfw_window, position.x, position.y);
        return *this;
    }

    const WindowMode& Window::mode()
    {
        return parameters._M_mode;
    }

    Window& Window::mode(const WindowMode& mode, const WindowSize& _size)
    {
        if (mode == parameters._M_mode)
            return *this;
        // [width, height, x, y]

        if (mode == NONE)
        {
            glfwSetWindowMonitor(glfw_window, nullptr, parameters._M_backup[0], parameters._M_backup[1], parameters._M_backup[2],
                                 parameters._M_backup[3], Monitor::getRefreshRate());
            parameters._M_backup.clear();
        }
        else
        {
            if (parameters._M_mode == NONE)
            {
                parameters._M_backup.clear();
                auto pos = position();
                auto _size = size();
                parameters._M_backup.push_back(pos.x);
                parameters._M_backup.push_back(pos.y);
                parameters._M_backup.push_back(_size.x);
                parameters._M_backup.push_back(_size.y);
            }
            glfwSetWindowMonitor(glfw_window, mode == FULLSCREEN ? Monitor::monitor : nullptr, 0, 0,
                                 _size.x == -1 ? Monitor::getWidth() : _size.x, _size.y == -1 ? Monitor::getHeight() : _size.x,
                                 Monitor::getRefreshRate());
        }
        parameters._M_mode = mode;
        return *this;
    }

    void Window::close()
    {
        CHECK(void);
        parameters._M_is_closed = true;
        glfwSetWindowShouldClose(glfw_window, true);
    }

    void Window::focus()
    {
        CHECK(void);
        if (glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED) == 0)
        {
            glfwFocusWindow(glfw_window);
        }
    }

    bool Window::focused()
    {
        return glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED) != 0;
    }

    void Window::hide()
    {
        CHECK(void);
        if (glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE) != 0)
            glfwHideWindow(glfw_window);
    }

    void Window::show()
    {
        CHECK(void);
        if (glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE) == 0)
            glfwShowWindow(glfw_window);
    }

    bool Window::is_visible()
    {
        return glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE) != 0;
    }

    Color& Window::background_color()
    {
        return parameters._M_color;
    }


    Window& Window::background_color(const Color& color)
    {
        make_current();
        glClearColor(color.R(), color.G(), color.B(), color.A());
        parameters._M_color = color;
        return *this;
    }

    bool Window::is_iconify()
    {
        return glfwGetWindowAttrib(glfw_window, GLFW_ICONIFIED) != 0;
    }

    Window& Window::iconify()
    {
        glfwIconifyWindow(glfw_window);
        return *this;
    }

    bool Window::is_restored()
    {
        return !is_iconify();
    }

    Window& Window::restore()
    {
        glfwRestoreWindow(glfw_window);
        return *this;
    }

    Window& Window::opacity(float value)
    {
        glfwSetWindowOpacity(glfw_window, value);
        return *this;
    }

    float Window::opacity()
    {
        return glfwGetWindowOpacity(glfw_window);
    }

    bool Window::center_cursor()
    {
        return glfwGetWindowAttrib(glfw_window, GLFW_CENTER_CURSOR) != 0;
    }

    Window& Window::center_cursor(bool value)
    {
        glfwSetWindowAttrib(glfw_window, GLFW_CENTER_CURSOR, value);
        return *this;
    }

    Window& Window::size_limits(const SizeLimits& limits)
    {
        glfwSetWindowSizeLimits(glfw_window, limits.min_size.x, limits.min_size.y, limits.max_size.x, limits.max_size.y);
        parameters._M_limits = limits;
        return *this;
    }

    const SizeLimits& Window::size_limits()
    {
        return parameters._M_limits;
    }

    void Window::use_default_cursor()
    {
        glfwSetCursor(glfw_window, nullptr);
    }

    Window& Window::aspect_ration(const AspectRation& ration)
    {
        glfwSetWindowAspectRatio(glfw_window, ration.numer, ration.denom);
        parameters._M_aspect_ration = ration;
        return *this;
    }

    const AspectRation& Window::aspect_ration()
    {
        return parameters._M_aspect_ration;
    }

    void Window::disable_aspect_ration()
    {
        glfwSetWindowAspectRatio(glfw_window, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }

    Window& Window::icon(const std::string& path)
    {
        glfwSetWindowIcon(glfw_window, 1, static_cast<GLFWimage*>(parameters._M_icon.load(path).add_alpha_channel().glfw_image()));
        return *this;
    }

    const Image& Window::icon()
    {
        return parameters._M_icon;
    }

    Window& Window::icon(const Image& icon)
    {
        parameters._M_icon = icon;
        glfwSetWindowIcon(glfw_window, 1, static_cast<GLFWimage*>(parameters._M_icon.add_alpha_channel().glfw_image()));
        return *this;
        glfwCreateCursor(nullptr, 0, 0);
    }

    Window& Window::cursor(const Cursor& cursor)
    {
        parameters._M_cursor = cursor;
        glfwSetCursor(glfw_window, static_cast<GLFWcursor*>(parameters._M_cursor.glfw_cursor()));
        return *this;
    }

    const Cursor& Window::cursor()
    {
        return parameters._M_cursor;
    }

    void Window::destroy()
    {
        if (parameters._M_is_closed == false)
        {
            std::clog << "Engine::Window: Window '" << parameters._M_name << "' closed,\taddress of window: " << this << std::endl;
            glfwDestroyWindow(glfw_window);
        }

        parameters._M_is_closed = true;
    }

    bool Window::vsync() const
    {
        return parameters._M_vsync;
    }

    Window& Window::vsync(const bool& value)
    {
        parameters._M_vsync = value;
        glfwSwapInterval(static_cast<int>(value));
        return *this;
    }


}// namespace Engine
