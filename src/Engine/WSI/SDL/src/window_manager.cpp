#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui_impl_sdl2.h>
#include <window_manager.hpp>
#include <window_system.hpp>

namespace Engine
{
    template<typename Type>
    using ValueMap = const Map<Uint8, Type>;

    static ValueMap<GameController::Axis> axis_type = {
            {SDL_CONTROLLER_AXIS_INVALID, GameController::Axis::None},
            {SDL_CONTROLLER_AXIS_LEFTX, GameController::Axis::LeftX},
            {SDL_CONTROLLER_AXIS_LEFTY, GameController::Axis::LeftY},
            {SDL_CONTROLLER_AXIS_TRIGGERLEFT, GameController::Axis::TriggerLeft},
            {SDL_CONTROLLER_AXIS_RIGHTX, GameController::Axis::RightX},
            {SDL_CONTROLLER_AXIS_RIGHTY, GameController::Axis::RightY},
            {SDL_CONTROLLER_AXIS_TRIGGERLEFT, GameController::Axis::TriggerRight},
    };

    static ValueMap<Mouse::Button> mouse_buttons = {{SDL_BUTTON_LEFT, Mouse::Button::Left},
                                                    {SDL_BUTTON_MIDDLE, Mouse::Button::Middle},
                                                    {SDL_BUTTON_RIGHT, Mouse::Button::Right},
                                                    {SDL_BUTTON_X1, Mouse::Button::X1},
                                                    {SDL_BUTTON_X2, Mouse::Button::X2}};

    static ValueMap<EventType> window_event_types = {
            {SDL_WINDOWEVENT_NONE, EventType::WindowNone},
            {SDL_WINDOWEVENT_SHOWN, EventType::WindowShown},
            {SDL_WINDOWEVENT_HIDDEN, EventType::WindowHidden},
            {SDL_WINDOWEVENT_EXPOSED, EventType::WindowExposed},
            {SDL_WINDOWEVENT_MOVED, EventType::WindowMoved},
            {SDL_WINDOWEVENT_RESIZED, EventType::WindowResized},
            {SDL_WINDOWEVENT_SIZE_CHANGED, EventType::WindowSizeChanged},
            {SDL_WINDOWEVENT_MINIMIZED, EventType::WindowMinimized},
            {SDL_WINDOWEVENT_MAXIMIZED, EventType::WindowMaximized},
            {SDL_WINDOWEVENT_RESTORED, EventType::WindowRestored},
            {SDL_WINDOWEVENT_ENTER, EventType::WindowEnter},
            {SDL_WINDOWEVENT_LEAVE, EventType::WindowLeave},
            {SDL_WINDOWEVENT_FOCUS_GAINED, EventType::WindowFocusGained},
            {SDL_WINDOWEVENT_FOCUS_LOST, EventType::WindowFocusLost},
            {SDL_WINDOWEVENT_CLOSE, EventType::WindowClose},
            {SDL_WINDOWEVENT_TAKE_FOCUS, EventType::WindowTakeFocus},
            {SDL_WINDOWEVENT_HIT_TEST, EventType::WindowHitTest},
            {SDL_WINDOWEVENT_ICCPROF_CHANGED, EventType::WindowIccProfChanged},
            {SDL_WINDOWEVENT_DISPLAY_CHANGED, EventType::WindowDisplayChanged},
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


    SDL2_WindowManagerInterface::SDL2_WindowManagerInterface()
    {
        SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO);
    }

    SDL2_WindowManagerInterface::~SDL2_WindowManagerInterface()
    {
        for (auto& pair : _M_game_controllers)
        {
            info_log("WindowSDL", "Force close controller with id %d", pair.first);
            SDL_GameControllerClose(pair.second);
        }

        _M_game_controllers.clear();

        SDL_Quit();
    }


    WindowInterface* SDL2_WindowManagerInterface::create_window(const WindowConfig* config)
    {
        return (new WindowSDL())->initialize(config);
    }


    WindowManagerInterface& SDL2_WindowManagerInterface::destroy_window(WindowInterface* interface)
    {
        delete interface;
        return *this;
    }

    void create_system_notify(const NotifyCreateInfo& info);

    WindowManagerInterface& SDL2_WindowManagerInterface::create_notify(const NotifyCreateInfo& info)
    {
        create_system_notify(info);
        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::start_text_input()
    {
        SDL_StartTextInput();
        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::stop_text_input()
    {
        SDL_StopTextInput();
        return *this;
    }

    String SDL2_WindowManagerInterface::error() const
    {
        return SDL_GetError();
    }

    bool SDL2_WindowManagerInterface::has_error() const
    {
        const char* msg = SDL_GetError();
        return std::strcmp(msg, "") != 0;
    }

    bool SDL2_WindowManagerInterface::mouse_relative_mode() const
    {
        return static_cast<bool>(SDL_GetRelativeMouseMode());
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::mouse_relative_mode(bool flag)
    {
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(flag));
        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::update_monitor_info(MonitorInfo& info)
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(0, &mode);
        info.height       = mode.h;
        info.width        = mode.w;
        info.refresh_rate = mode.refresh_rate;
        SDL_GetDisplayDPI(0, &info.dpi.ddpi, &info.dpi.hdpi, &info.dpi.vdpi);
        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::add_event_callback(Identifier system_id,
                                                                            const EventCallback& callback)
    {
        _M_event_callbacks[system_id].push_back(callback);
        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::remove_all_callbacks(Identifier system_id)
    {
        _M_event_callbacks.erase(system_id);
        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::pool_events()
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();

        while (SDL_PollEvent(&_M_event))
        {
            process_event();
        }

        ImGui::SetCurrentContext(ctx);

        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::wait_for_events()
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        SDL_WaitEvent(&_M_event);
        process_event();
        ImGui::SetCurrentContext(ctx);
        return *this;
    }


#define new_event(a, b) send_event(Event(_M_event.window.windowID, EventType::a, b))
#define new_event_typed(a, b) send_event(Event(_M_event.window.windowID, a, b))

    void SDL2_WindowManagerInterface::send_event(const Event& event)
    {
        for (auto& system_callbacks : _M_event_callbacks)
        {
            for (auto& callback : system_callbacks.second)
            {
                callback(event);
            }
        }
    }

    class RenderThreadImGuiEvent : public ExecutableObject
    {
        SDL_Event _M_event;
        ImGuiContext* _M_ctx;

    public:
        RenderThreadImGuiEvent(const SDL_Event& event, ImGuiContext* ctx) : _M_event(event), _M_ctx(ctx)
        {}

        int_t execute() override
        {
            ImGuiContext* current_ctx = ImGui::GetCurrentContext();
            ImGui::SetCurrentContext(_M_ctx);

            ImGui_ImplSDL2_ProcessEvent(&_M_event);
            ImGui::SetCurrentContext(current_ctx);

            return sizeof(RenderThreadImGuiEvent);
        }
    };


    void SDL2_WindowManagerInterface::process_imgui_event()
    {
        Window* window = WindowManager::instance()->find(_M_event.window.windowID);

        if (!window)
        {
            return;
        }

        auto imgui_window = window->imgui_window();
        if (!imgui_window)
            return;

        ImGuiContext* imgui_context = imgui_window->context();

        if (imgui_context)
        {
            ImGui::SetCurrentContext(imgui_context);
            ImGui_ImplSDL2_ProcessEvent(&_M_event);
        }
    }

    void SDL2_WindowManagerInterface::process_event()
    {
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

                    engine_event.x = _M_event.window.data1;
                    engine_event.y = _M_event.window.data2;

                    new_event_typed(it->second, engine_event);
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
                int w, h;
                SDL_GetWindowSize(SDL_GetWindowFromID(_M_event.window.windowID), &w, &h);

                MouseMotionEvent mouse_motion;
                mouse_motion.x    = _M_event.motion.x;
                mouse_motion.y    = h - _M_event.motion.y;
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
                _M_game_controllers[_M_event.cdevice.which] = SDL_GameControllerOpen(_M_event.cdevice.which);
                ControllerDeviceAddedEvent c_event;
                c_event.id = static_cast<Identifier>(_M_event.cdevice.which) + 1;
                new_event(ControllerDeviceAdded, c_event);
                break;
            }

            case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_GameControllerClose(_M_game_controllers[_M_event.cdevice.which]);
                _M_game_controllers.erase(_M_event.cdevice.which);
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


        process_imgui_event();
    }


    void SDL2_WindowManagerInterface::process_mouse_button()
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
}// namespace Engine


TRINEX_EXTERNAL_LIB_INIT_FUNC(Engine::WindowManagerInterface*)
{
    return new Engine::SDL2_WindowManagerInterface();
}
