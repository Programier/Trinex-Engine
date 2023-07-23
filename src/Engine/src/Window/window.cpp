#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/g_buffer.hpp>
#include <SDL.h>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <api.hpp>

#include <sdl_surface.hpp>


using namespace Engine;


#define win_init_error throw EngineException("Init window first")

#define check_init(value)                                                                                              \
    if (!_M_is_inited)                                                                                                 \
    return value


static const Map<WindowAttrib, SDL_WindowFlags> window_attributes = {
        {WindowAttrib::WinResizable, SDL_WINDOW_RESIZABLE},
        {WindowAttrib::WinFullScreen, SDL_WINDOW_FULLSCREEN},
        {WindowAttrib::WinFullScreenDesktop, SDL_WINDOW_FULLSCREEN_DESKTOP},
        {WindowAttrib::WinShown, SDL_WINDOW_SHOWN},
        {WindowAttrib::WinHidden, SDL_WINDOW_HIDDEN},
        {WindowAttrib::WinBorderLess, SDL_WINDOW_BORDERLESS},
        {WindowAttrib::WinMouseFocus, SDL_WINDOW_MOUSE_FOCUS},
        {WindowAttrib::WinInputFocus, SDL_WINDOW_INPUT_FOCUS},
        {WindowAttrib::WinInputGrabbed, SDL_WINDOW_INPUT_GRABBED},
        {WindowAttrib::WinMinimized, SDL_WINDOW_MINIMIZED},
        {WindowAttrib::WinMaximized, SDL_WINDOW_MAXIMIZED},
        {WindowAttrib::WinMouseCapture, SDL_WINDOW_MOUSE_CAPTURE},
        {WindowAttrib::WinAllowHighDPI, SDL_WINDOW_ALLOW_HIGHDPI},
        {WindowAttrib::WinMouseGrabbed, SDL_WINDOW_MOUSE_GRABBED},
        {WindowAttrib::WinKeyboardGrabbed, SDL_WINDOW_KEYBOARD_GRABBED}};


static const WindowAttrib attrib_list[] = {
        WinResizable,    WinFullScreen,   WinFullScreenDesktop, WinShown,
        WinHidden,       WinBorderLess,   WinMouseFocus,        WinInputFocus,
        WinInputGrabbed, WinMinimized,    WinMaximized,         WinTransparentFramebuffer,
        WinMouseCapture, WinAllowHighDPI, WinMouseGrabbed,      WinKeyboardGrabbed};

static constexpr int_t attributes_count = sizeof(attrib_list) / sizeof(WindowAttrib);

static List<WindowAttrib> parse_win_attibutes(uint16_t attrib)
{
    List<WindowAttrib> attributes;
    for (int_t i = 0; i < attributes_count; i++)
    {
        if (attrib & (1 << i))
            attributes.push_back(attrib_list[i]);
    }
    return attributes;
}

static uint32_t to_sdl_attrib(const List<WindowAttrib>& attrib)
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


Window* Window::_M_instance = nullptr;
Window* Window::window      = nullptr;

Window* Window::free_icon_surface()
{
    SDL_SetWindowIcon(_M_window, nullptr);
    if (_M_icon_surface)
    {
        info_log("Window", "Destroy icon surface\n");
        SDL_FreeSurface(_M_icon_surface);
    }
    _M_icon_surface = nullptr;
    return this;
}

//          WINDOW INITIALIZATION


static void window_initialize_error(const String& msg)
{
    info_log("Window", "Failed to initialize new window, error: '%s'\n", msg.c_str());
    Window::instance()->close();
    throw EngineException("Failed to create Window");
}

Window* Window::init(float width, float height, const String& title, uint16_t attributes)
{
    if (_M_is_inited)
        return Window::_M_instance;

    if (!EngineInstance::instance()->is_inited())
        throw EngineException("Init Engine first");

    if (engine_instance->api() == EngineAPI::NoAPI)
        throw EngineException("Cannot create window without API!");

    _M_limits.max = Monitor::size();

    info_log("Window", "Creating new window '%s'\n", title.c_str());

    uint32_t attrib = to_sdl_attrib(parse_win_attibutes(attributes));
    _M_title        = title;

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, _M_X11_compositing);


    auto sdl_window_api =
            (EngineInstance::instance()->api() == EngineAPI::OpenGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN);

    _M_window = SDL_CreateWindow(_M_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 static_cast<int>(width), static_cast<int>(height),
                                 sdl_window_api | SDL_WINDOW_SHOWN | attrib);

    if (_M_window == nullptr)
        window_initialize_error(SDL_GetError());

    _M_size = {width, height};
    Window::size_limits(_M_limits);

    _M_is_inited = true;
    _M_flags     = SDL_GetWindowFlags(_M_window);

    return Window::_M_instance;
}

Window* Window::init(const Size2D& size, const String& title, uint16_t attrib)
{
    return init(size.x, size.y, title, attrib);
}


//          CLOSING WINDOW

Window* Window::destroy_window()
{
    if (_M_window)
    {
        free_icon_surface();
        EngineInstance::instance()->api_interface()->destroy_window();
        SDL_DestroyWindow(_M_window);
    }

    return Window::_M_instance;
}

Window* Window::close()
{
    check_init(Window::_M_instance);

    info_log("Window", "Closing window\n");
    _M_is_inited = false;
    SDL_HideWindow(_M_window);
    info_log("Engine::Window: Window '%s' closed\n", _M_title.c_str());

    return Window::_M_instance;
}


bool Window::is_open() const
{
    return _M_is_inited;
}


const Window* Window::swap_buffers() const
{
    check_init(Window::_M_instance);
    EngineInstance::instance()->api_interface()->swap_buffer(_M_window);
    GBuffer* buffer = GBuffer::instance();
    if (buffer)
    {
        buffer->swap_buffer();
    }
    return Window::_M_instance;
}


//          CHANGING SIZE OF WINDOW

Size1D Window::width() const
{
    return _M_size.x;
}

const Window* Window::width(const Size1D& width) const
{
    check_init(Window::_M_instance);
    SDL_SetWindowSize(_M_window, static_cast<int>(width), static_cast<int>(_M_size.y));
    return Window::_M_instance;
}

Size1D Window::height() const
{
    return _M_size.y;
}

const Window* Window::height(const Size1D& height) const
{
    check_init(Window::_M_instance);
    SDL_SetWindowSize(_M_window, static_cast<int>(_M_size.x), static_cast<int>(height));
    return Window::_M_instance;
}

const Size2D& Window::size() const
{
    return _M_size;
}

const Window* Window::size(const Size2D& size) const
{
    check_init(Window::_M_instance);
    SDL_SetWindowSize(_M_window, static_cast<int>(size.x), static_cast<int>(size.y));
    return Window::_M_instance;
}


void Window::process_window_event(SDL_WindowEvent& event)
{
    switch (event.event)
    {
        case SDL_WINDOWEVENT_MOVED:
        {
            _M_position = {event.data1, event.data2};
            break;
        }

        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESIZED:
        {
            EngineInstance::instance()->api_interface()->on_window_size_changed();
            _M_size = {event.data1, event.data2};
            if (_M_change_viewport_on_resize)
                update_view_port();
            if (_M_update_scissor_on_resize)
                update_scissor();

            Window::on_resize.trigger();
            break;
        }

        case SDL_WINDOWEVENT_ENTER:
        {
            break;
        }


        case SDL_WINDOWEVENT_CLOSE:
            _M_is_inited = false;
            break;
    }
}


void Window::process_paths_event(SDL_DropEvent& event)
{
    if (event.file)
    {
        _M_dropped_paths.push_back(event.file);
        SDL_free(event.file);
    }
}


bool Window::vsync() const
{
    return _M_swap_interval > 0;
}

Window* Window::vsync(bool value)
{
    _M_swap_interval = static_cast<int>(value);
    return swap_interval(static_cast<int>(value));
}

int_t Window::swap_interval() const
{
    return _M_swap_interval;
}

Window* Window::swap_interval(int_t value)
{
    check_init(Window::_M_instance);
    _M_swap_interval = value;
    EngineInstance::instance()->api_interface()->swap_interval(value);
    return Window::_M_instance;
}


const String& Window::title() const
{
    return _M_title;
}

Window* Window::title(const String& title)
{

    check_init(Window::_M_instance);
    _M_title = title;
    SDL_SetWindowTitle(_M_window, _M_title.c_str());
    return Window::_M_instance;
}


const Point2D& Window::position() const
{
    return _M_position;
}


const Window* Window::position(const Point2D& position) const
{
    if (_M_is_inited)
        SDL_SetWindowPosition(_M_window, static_cast<int>(position.x), static_cast<int>(position.y));
    return Window::_M_instance;
}


const Vector<String>& Window::dropped_paths() const
{
    return _M_dropped_paths;
}


Window* Window::clear_dropped_paths()
{
    _M_dropped_paths.clear();
    return Window::_M_instance;
}

bool Window::rezisable() const
{
    return static_cast<bool>(_M_flags & SDL_WINDOW_RESIZABLE);
}

const Window* Window::rezisable(bool value) const
{
    check_init(Window::_M_instance);
    SDL_SetWindowResizable(_M_window, static_cast<SDL_bool>(value));
    return Window::_M_instance;
}


const Window* Window::focus() const
{
    check_init(Window::_M_instance);
    SDL_SetWindowInputFocus(_M_window);
    return Window::_M_instance;
}

bool Window::focused() const
{
    check_init(false);
    return static_cast<bool>(_M_flags & SDL_WINDOW_INPUT_FOCUS);
}

const Window* Window::show() const
{
    check_init(Window::_M_instance);
    SDL_ShowWindow(_M_window);
    return Window::_M_instance;
}

const Window* Window::hide() const
{
    check_init(Window::_M_instance);
    SDL_HideWindow(_M_window);
    return Window::_M_instance;
}

bool Window::is_visible() const
{
    check_init(false);
    return static_cast<bool>(_M_flags & SDL_WINDOW_SHOWN);
}


bool Window::is_iconify() const
{
    check_init(false);
    return static_cast<bool>(_M_flags & SDL_WINDOW_MINIMIZED);
}

const Window* Window::iconify() const
{
    check_init(Window::_M_instance);
    SDL_MinimizeWindow(_M_window);
    return Window::_M_instance;
}

bool Window::is_restored() const
{
    check_init(false);
    return !is_iconify();
}

const Window* Window::restore() const
{
    check_init(Window::_M_instance);
    SDL_RestoreWindow(_M_window);
    return Window::_M_instance;
}


const Window* Window::opacity(float value) const
{
    check_init(Window::_M_instance);
    SDL_SetWindowOpacity(_M_window, value);
    return Window::_M_instance;
}

float Window::opacity() const
{
    check_init(0.f);
    float _M_opacity = 0.f;
    SDL_GetWindowOpacity(_M_window, &_M_opacity);
    return _M_opacity;
}


Window* Window::size_limits(const SizeLimits2D& limits)
{
    check_init(Window::_M_instance);
    _M_limits = limits;
    SDL_SetWindowMaximumSize(_M_window, static_cast<int>(_M_limits.max.x), static_cast<int>(_M_limits.max.y));
    SDL_SetWindowMinimumSize(_M_window, static_cast<int>(_M_limits.min.x), static_cast<int>(_M_limits.min.y));
    return Window::_M_instance;
}

const SizeLimits2D& Window::size_limits() const
{
    return _M_limits;
}

const Cursor& Window::cursor() const
{
    return _M_cursor;
}

Window* Window::cursor(const Cursor& cursor)
{
    _M_cursor = cursor;
    SDL_SetCursor(static_cast<SDL_Cursor*>(_M_cursor.sdl_cursor()));
    return Window::_M_instance;
}

Window* Window::icon(const Image& image)
{
    check_init(Window::_M_instance);
    _M_icon = image;
    _M_icon.add_alpha_channel();
    free_icon_surface();
    _M_icon_surface = create_sdl_surface(_M_icon);
    SDL_SetWindowIcon(_M_window, _M_icon_surface);
    return Window::_M_instance;
}

Window* Window::icon(const String& image)
{
    check_init(Window::_M_instance);
    _M_icon.load(image);
    free_icon_surface();
    _M_icon_surface = create_sdl_surface(_M_icon);
    SDL_SetWindowIcon(_M_window, _M_icon_surface);
    return Window::_M_instance;
}

const Image& Window::icon() const
{
    return _M_icon;
}

Window* Window::attribute(const WindowAttrib& attrib, bool value)
{
    check_init(Window::_M_instance);
    _M_flags = SDL_GetWindowFlags(_M_window);
    try
    {
        auto list = parse_win_attibutes(attrib);

        static struct {
            bool _M_fullscreen = false;
            Uint32 _M_flag     = 0;
        } fullscreen_mode;

        fullscreen_mode._M_fullscreen = false;
        fullscreen_mode._M_flag       = 0;

        for (auto& attrib : list)
        {

            switch (attrib)
            {
                case WindowAttrib::WinResizable:
                    SDL_SetWindowResizable(_M_window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttrib::WinFullScreen:
                    fullscreen_mode._M_flag       = value ? SDL_WINDOW_FULLSCREEN : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttrib::WinFullScreenDesktop:
                    fullscreen_mode._M_flag       = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                    fullscreen_mode._M_fullscreen = true;
                    break;

                case WindowAttrib::WinShown:
                {
                    value ? show() : hide();
                    break;
                }
                case WindowAttrib::WinHidden:
                {
                    value ? hide() : show();
                    break;
                }
                case WindowAttrib::WinBorderLess:
                    SDL_SetWindowBordered(_M_window, static_cast<SDL_bool>(value));
                    break;

                case WindowAttrib::WinInputFocus:
                {
                    if (value)
                        SDL_SetWindowInputFocus(_M_window);
                    break;
                }

                case WindowAttrib::WinMinimized:
                {
                    if (value)
                        SDL_MinimizeWindow(_M_window);
                    break;
                }

                case WindowAttrib::WinMaximized:
                {
                    if (value)
                        SDL_MaximizeWindow(_M_window);
                    break;
                }
                case WindowAttrib::WinMouseGrabbed:
                {
                    SDL_SetWindowMouseGrab(_M_window, static_cast<SDL_bool>(value));
                    break;
                }

                case WindowAttrib::WinKeyboardGrabbed:
                {
                    SDL_SetWindowKeyboardGrab(_M_window, static_cast<SDL_bool>(value));
                    break;
                }

                case WindowAttrib::WinTransparentFramebuffer:
                {
                    if (_M_is_inited)
                        info_log("Window", "Cannot change flag WIN_TRANSPARENT_FRAMEBUFFER after creating window\n");
                    else
                        _M_is_transparent_framebuffer = value;
                    break;
                }

                default:
                    break;
            }
        }

        if (fullscreen_mode._M_fullscreen)
            SDL_SetWindowFullscreen(_M_window, fullscreen_mode._M_flag);
    }
    catch (const std::exception& e)
    {
        info_log("Window", "%s\n", e.what());
    }

    _M_flags = SDL_GetWindowFlags(_M_window);
    return Window::_M_instance;
}

bool Window::attribute(const WindowAttrib& attrib) const
{
    check_init(false);
    try
    {
        auto attribute_list = parse_win_attibutes(attrib);
        if (attribute_list.size() != 1)
            throw std::runtime_error("Failed to get attibute");
        auto attrib = attribute_list.front();
        if (attrib == WindowAttrib::WinTransparentFramebuffer)
            return _M_is_transparent_framebuffer;
        return static_cast<bool>(_M_flags & window_attributes.at(attrib));
    }
    catch (const std::exception& e)
    {
        info_log("%s\n", e.what());
        return false;
    }
}


Window* Window::X11_compositing(bool value)
{
    _M_X11_compositing = (value ? "0" : "1");
    return Window::_M_instance;
}


void* Window::SDL() const
{
    return static_cast<void*>(_M_window);
}

void* Window::api_context() const
{
    return _M_api_context;
}


Window* Window::cursor_mode(const CursorMode& mode)
{
    if (SDL_ShowCursor((mode == CursorMode::Hidden ? SDL_DISABLE : SDL_ENABLE)) < 0)
        Engine::info_log("Window", SDL_GetError());
    _M_cursor_mode = mode;
    return Window::_M_instance;
}

CursorMode Window::cursor_mode() const
{
    return _M_cursor_mode;
}

Window* Window::update_view_port()
{
    check_init(Window::_M_instance);
    _M_viewport.pos  = {0, 0};
    _M_viewport.size = size();
    view_port(_M_viewport);

    return Window::_M_instance;
}

Window* Window::update_scissor()
{
    check_init(Window::_M_instance);
    _M_scissor.pos  = {0, 0};
    _M_scissor.size = size();
    scissor(_M_scissor);
    return Window::_M_instance;
}

Window* Window::set_orientation(uint_t orientation)
{
    static Map<WindowOrientation, const char*> _M_orientation_map = {
            {WindowOrientation::WinOrientationLandscape, "LandscapeRight"},
            {WindowOrientation::WinOrientationLandscapeFlipped, "LandscapeLeft"},
            {WindowOrientation::WinOrientationPortrait, "Portrait"},
            {WindowOrientation::WinOrientationPortraitFlipped, "PortraitUpsideDown"}};

    static WindowOrientation orientations[] = {WinOrientationPortrait, WinOrientationPortraitFlipped,
                                               WinOrientationLandscape, WinOrientationLandscapeFlipped};

    if (_M_is_inited)
    {
        info_log("Window", "Can't change orientation after creating window\n");
        return Window::_M_instance;
    }

    String result;
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
    return Window::_M_instance;
}

const Window* Window::start_text_input() const
{
    SDL_StartTextInput();
    return Window::_M_instance;
}

const Window* Window::stop_text_input() const
{
    SDL_StopTextInput();
    return Window::_M_instance;
}

Window* Window::update_viewport_on_resize(bool value)
{
    _M_change_viewport_on_resize = value;
    return Window::_M_instance;
}

bool Window::update_viewport_on_resize() const
{
    return _M_change_viewport_on_resize;
}

Window* Window::update_scissor_on_resize(bool value)
{
    _M_update_scissor_on_resize = value;
    return Window::_M_instance;
}

bool Window::update_scissor_on_resize() const
{
    return _M_update_scissor_on_resize;
}


Window* Window::initialize_api()
{
    if (!is_api_initialized())
    {
        _M_api_context = EngineInstance::instance()->api_interface()->init_window(_M_window);
        if (_M_api_context == nullptr)
        {
            window_initialize_error("Failed to initialize API!");
        }

        update_view_port()->update_scissor();
        swap_interval(1);

        if (engine_config.enable_g_buffer && GBuffer::instance() == nullptr)
        {
            GBuffer::init_g_buffer();
        }
        _M_api_inited = true;
    }

    return this;
}

bool Window::is_api_initialized() const
{
    return _M_api_inited;
}

size_t Window::frame_number()
{
    return event.frame_number();
}

// Window constructors

Window::Window()
{
    window                    = this;
    _M_force_destroy_priority = Constants::max_size;
}

Window::~Window()
{
    destroy_window();
    _M_instance = nullptr;
    window      = nullptr;
}


static void on_init()
{
    {
        Lua::Namespace _namespace = Lua::Interpretter::namespace_of("Engine::");


        _namespace.new_enum<WindowAttrib>("WindowAttrib", {{"WinNone", WindowAttrib::WinKeyboardGrabbed},
                                                           {"WinResizable", WindowAttrib::WinMouseGrabbed},
                                                           {"WinFullScreen", WindowAttrib::WinAllowHighDPI},
                                                           {"WinFullScreenDesktop", WindowAttrib::WinMouseCapture},
                                                           {"WinShown", WindowAttrib::WinTransparentFramebuffer},
                                                           {"WinHidden", WindowAttrib::WinMaximized},
                                                           {"WinBorderLess", WindowAttrib::WinMinimized},
                                                           {"WinMouseFocus", WindowAttrib::WinInputGrabbed},
                                                           {"WinInputFocus", WindowAttrib::WinInputFocus},
                                                           {"WinInputGrabbed", WindowAttrib::WinMouseFocus},
                                                           {"WinMinimized", WindowAttrib::WinBorderLess},
                                                           {"WinMaximized", WindowAttrib::WinHidden},
                                                           {"WinTransparentFramebuffer", WindowAttrib::WinShown},
                                                           {"WinMouseCapture", WindowAttrib::WinFullScreenDesktop},
                                                           {"WinAllowHighDPI", WindowAttrib::WinFullScreen},
                                                           {"WinMouseGrabbed", WindowAttrib::WinResizable},
                                                           {"WinKeyboardGrabbed", WindowAttrib::WinNone}});


        _namespace.new_enum<CursorMode>("CursorMode", {{"Normal", CursorMode::Normal}, {"Hidden", CursorMode::Hidden}});


        _namespace.new_enum<WindowOrientation>(
                "WindowOrientation",
                {{"WinOrientationLandscape", WindowOrientation::WinOrientationLandscape},
                 {"WinOrientationLandscapeFlipped", WindowOrientation::WinOrientationLandscapeFlipped},
                 {"WinOrientationPortrait", WindowOrientation::WinOrientationPortrait},
                 {"WinOrientationPortraitFlipped", WindowOrientation::WinOrientationPortraitFlipped}});
    }

    Engine::Class::register_new_class<Engine::Window>("Engine::Window")
            .get()
            .set("init", Lua::overload(func_of<Window*, Window, float>(&Window::init),
                                       func_of<Window*, Window, const Size2D&, const String&, uint16_t>(&Window::init)))
            .set("close", &Window::close)
            .set("is_open", &Window::is_open)
            .set("swap_buffers", &Window::swap_buffers)
            .set("width", Lua::overload(func_of<Size1D, Window>(&Window::width),
                                        func_of<const Window*, Window, const Size1D&>(&Window::width)))

            .set("height", Lua::overload(func_of<Size1D>(&Window::height),
                                         func_of<const Window*, Window, const Size1D&>(&Window::height)))

            .set("size", Lua::overload(func_of<const Size2D&>(&Window::size), func_of<const Window*>(&Window::size)))
            .set("swap_interval",
                 Lua::overload(func_of<int_t>(&Window::swap_interval), func_of<Window*>(&Window::swap_interval)))
            .set("vsync", Lua::overload(func_of<bool>(&Window::vsync), func_of<Window*>(&Window::vsync)))
            .set("title", Lua::overload(func_of<const String&>(&Window::title), func_of<Window*>(&Window::title)))
            .set("position",
                 Lua::overload(func_of<const Point2D&>(&Window::position), func_of<const Window*>(&Window::position)))
            .set("dropped_paths", &Window::dropped_paths)
            .set("clear_dropped_paths", &Window::clear_dropped_paths)
            .set("rezisable",
                 Lua::overload(func_of<bool>(&Window::rezisable), func_of<const Window*>(&Window::rezisable)))
            .set("focus", &Window::focus)
            .set("focused", &Window::focused)
            .set("show", &Window::show)
            .set("hide", &Window::hide)
            .set("is_visible", &Window::is_visible)
            .set("is_iconify", &Window::is_iconify)
            .set("iconify", &Window::iconify)
            .set("is_restored", &Window::is_restored)
            .set("restore", &Window::restore)
            .set("opacity", Lua::overload(func_of<float>(&Window::opacity), func_of<const Window*>(&Window::opacity)))
            .set("attribute", Lua::overload(func_of<bool>(&Window::attribute), func_of<Window*>(&Window::attribute)))
            .set("cursor_mode",
                 Lua::overload(func_of<CursorMode>(&Window::cursor_mode), func_of<Window*>(&Window::cursor_mode)))
            .set("update_view_port", &Window::update_view_port)
            .set("update_scissor", &Window::update_scissor)
            .set("X11_compositing", &Window::X11_compositing)
            .set("SDL", &Window::SDL)
            .set("api_context", &Window::api_context)
            .set("set_orientation", &Window::set_orientation)
            .set("start_text_input", &Window::start_text_input)
            .set("stop_text_input", &Window::stop_text_input)
            .set("update_viewport_on_resize", Lua::overload(func_of<bool>(&Window::update_viewport_on_resize),
                                                            func_of<Window*>(&Window::update_viewport_on_resize)))
            .set("update_scissor_on_resize", Lua::overload(func_of<bool>(&Window::update_scissor_on_resize),
                                                           func_of<Window*>(&Window::update_scissor_on_resize)))
            .set("frame_number", &Window::frame_number)
            .set("initialize_api", &Window::initialize_api)
            .set("window", Lua::property([]() -> Window* { return Window::window; }));
}

static InitializeController init_window(on_init);
