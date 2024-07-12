#include <Core/logger.hpp>
#include <Core/struct.hpp>
#include <EGL/egl.h>
#include <Event/event_data.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/event_system.hpp>
#include <Systems/touchscreen_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <android_platform.hpp>
#include <android_window.hpp>
#include <imgui_impl_android.h>

namespace Engine::Platform
{

    static bool m_is_inited           = false;
    static android_app* m_application = nullptr;
    static AndroidWindow* m_window    = nullptr;

    template<typename T>
    using ValueMap = Map<int32_t, T>;


    static ValueMap<Keyboard::Key> keys = {
            {AKEYCODE_UNKNOWN, Keyboard::Key::Unknown},
            {AKEYCODE_SPACE, Keyboard::Key::Space},
            {AKEYCODE_APOSTROPHE, Keyboard::Key::Apostrophe},
            {AKEYCODE_COMMA, Keyboard::Key::Comma},
            {AKEYCODE_MINUS, Keyboard::Key::Minus},
            {AKEYCODE_PERIOD, Keyboard::Key::Period},
            {AKEYCODE_SLASH, Keyboard::Key::Slash},
            {AKEYCODE_0, Keyboard::Key::Num0},
            {AKEYCODE_1, Keyboard::Key::Num1},
            {AKEYCODE_2, Keyboard::Key::Num2},
            {AKEYCODE_3, Keyboard::Key::Num3},
            {AKEYCODE_4, Keyboard::Key::Num4},
            {AKEYCODE_5, Keyboard::Key::Num5},
            {AKEYCODE_6, Keyboard::Key::Num6},
            {AKEYCODE_7, Keyboard::Key::Num7},
            {AKEYCODE_8, Keyboard::Key::Num8},
            {AKEYCODE_9, Keyboard::Key::Num9},
            {AKEYCODE_SEMICOLON, Keyboard::Key::Semicolon},
            {AKEYCODE_EQUALS, Keyboard::Key::Equal},
            {AKEYCODE_A, Keyboard::Key::A},
            {AKEYCODE_B, Keyboard::Key::B},
            {AKEYCODE_C, Keyboard::Key::C},
            {AKEYCODE_D, Keyboard::Key::D},
            {AKEYCODE_E, Keyboard::Key::E},
            {AKEYCODE_F, Keyboard::Key::F},
            {AKEYCODE_G, Keyboard::Key::G},
            {AKEYCODE_H, Keyboard::Key::H},
            {AKEYCODE_I, Keyboard::Key::I},
            {AKEYCODE_J, Keyboard::Key::J},
            {AKEYCODE_K, Keyboard::Key::K},
            {AKEYCODE_L, Keyboard::Key::L},
            {AKEYCODE_M, Keyboard::Key::M},
            {AKEYCODE_N, Keyboard::Key::N},
            {AKEYCODE_O, Keyboard::Key::O},
            {AKEYCODE_P, Keyboard::Key::P},
            {AKEYCODE_Q, Keyboard::Key::Q},
            {AKEYCODE_R, Keyboard::Key::R},
            {AKEYCODE_S, Keyboard::Key::S},
            {AKEYCODE_T, Keyboard::Key::T},
            {AKEYCODE_U, Keyboard::Key::U},
            {AKEYCODE_V, Keyboard::Key::V},
            {AKEYCODE_W, Keyboard::Key::W},
            {AKEYCODE_X, Keyboard::Key::X},
            {AKEYCODE_Y, Keyboard::Key::Y},
            {AKEYCODE_Z, Keyboard::Key::Z},
            {AKEYCODE_LEFT_BRACKET, Keyboard::Key::LeftBracket},
            {AKEYCODE_BACKSLASH, Keyboard::Key::Backslash},
            {AKEYCODE_RIGHT_BRACKET, Keyboard::Key::RightBracket},
            {AKEYCODE_GRAVE, Keyboard::Key::GraveAccent},
            {AKEYCODE_EXPLORER, Keyboard::Key::Explorer},
            {AKEYCODE_ESCAPE, Keyboard::Key::Escape},
            {AKEYCODE_ENTER, Keyboard::Key::Enter},
            {AKEYCODE_TAB, Keyboard::Key::Tab},
            {AKEYCODE_BACKSLASH, Keyboard::Key::Backspace},
            {AKEYCODE_INSERT, Keyboard::Key::Insert},
            {AKEYCODE_DEL, Keyboard::Key::Delete},
            {AKEYCODE_DPAD_RIGHT, Keyboard::Key::Right},
            {AKEYCODE_DPAD_LEFT, Keyboard::Key::Left},
            {AKEYCODE_DPAD_DOWN, Keyboard::Key::Down},
            {AKEYCODE_DPAD_UP, Keyboard::Key::Up},
            {AKEYCODE_PAGE_UP, Keyboard::Key::PageUp},
            {AKEYCODE_PAGE_DOWN, Keyboard::Key::PageDown},
            {AKEYCODE_HOME, Keyboard::Key::Home},
            {AKEYCODE_MOVE_END, Keyboard::Key::End},
            {AKEYCODE_CAPS_LOCK, Keyboard::Key::CapsLock},
            {AKEYCODE_SCROLL_LOCK, Keyboard::Key::ScrollLock},
            {AKEYCODE_NUM_LOCK, Keyboard::Key::NumLock},
            {AKEYCODE_SYSRQ, Keyboard::Key::PrintScreen},
            {AKEYCODE_MEDIA_PAUSE, Keyboard::Key::Pause},
            {AKEYCODE_F1, Keyboard::Key::F1},
            {AKEYCODE_F2, Keyboard::Key::F2},
            {AKEYCODE_F3, Keyboard::Key::F3},
            {AKEYCODE_F4, Keyboard::Key::F4},
            {AKEYCODE_F5, Keyboard::Key::F5},
            {AKEYCODE_F6, Keyboard::Key::F6},
            {AKEYCODE_F7, Keyboard::Key::F7},
            {AKEYCODE_F8, Keyboard::Key::F8},
            {AKEYCODE_F9, Keyboard::Key::F9},
            {AKEYCODE_F10, Keyboard::Key::F10},
            {AKEYCODE_F11, Keyboard::Key::F11},
            {AKEYCODE_F12, Keyboard::Key::F12},
            {AKEYCODE_NUMPAD_0, Keyboard::Key::Kp0},
            {AKEYCODE_NUMPAD_1, Keyboard::Key::Kp1},
            {AKEYCODE_NUMPAD_2, Keyboard::Key::Kp2},
            {AKEYCODE_NUMPAD_3, Keyboard::Key::Kp3},
            {AKEYCODE_NUMPAD_4, Keyboard::Key::Kp4},
            {AKEYCODE_NUMPAD_5, Keyboard::Key::Kp5},
            {AKEYCODE_NUMPAD_6, Keyboard::Key::Kp6},
            {AKEYCODE_NUMPAD_7, Keyboard::Key::Kp7},
            {AKEYCODE_NUMPAD_8, Keyboard::Key::Kp8},
            {AKEYCODE_NUMPAD_9, Keyboard::Key::Kp9},
            {AKEYCODE_NUMPAD_DOT, Keyboard::Key::KpDot},
            {AKEYCODE_NUMPAD_DIVIDE, Keyboard::Key::KpDivide},
            {AKEYCODE_NUMPAD_MULTIPLY, Keyboard::Key::KpMultiply},
            {AKEYCODE_NUMPAD_ADD, Keyboard::Key::KpSubtract},
            {AKEYCODE_NUMPAD_SUBTRACT, Keyboard::Key::KpAdd},
            {AKEYCODE_NUMPAD_ENTER, Keyboard::Key::KpEnter},
            {AKEYCODE_NUMPAD_EQUALS, Keyboard::Key::KpEqual},
            {AKEYCODE_SHIFT_LEFT, Keyboard::Key::LeftShift},
            {AKEYCODE_CTRL_LEFT, Keyboard::Key::LeftControl},
            {AKEYCODE_ALT_LEFT, Keyboard::Key::LeftAlt},
            {AKEYCODE_META_LEFT, Keyboard::Key::LeftSuper},
            {AKEYCODE_SHIFT_RIGHT, Keyboard::Key::RightShift},
            {AKEYCODE_CTRL_RIGHT, Keyboard::Key::RightControl},
            {AKEYCODE_ALT_RIGHT, Keyboard::Key::RightAlt},
            {AKEYCODE_META_RIGHT, Keyboard::Key::RightSuper},
            {AKEYCODE_MENU, Keyboard::Key::Menu},
    };

    static FORCE_INLINE Identifier window_id()
    {
        if (m_window)
        {
            return m_window->id();
        }

        return 0;
    }

    static void handle_app_cmd(struct android_app* app, int32_t cmd)
    {
        EventSystem* event_system = m_is_inited ? EventSystem::instance() : nullptr;

        if (event_system == nullptr)
        {
            if (cmd == APP_CMD_INIT_WINDOW)
            {
                m_is_inited = true;
            }
            return;
        }
        else
        {
            switch (cmd)
            {
                case APP_CMD_TERM_WINDOW:
                    event_system->push_event(Event(window_id(), EventType::AppTerminating, AppTerminatingEvent()));
                    break;

                case APP_CMD_WINDOW_RESIZED:
                    break;

                case APP_CMD_CONTENT_RECT_CHANGED:
                    break;

                case APP_CMD_GAINED_FOCUS:
                    event_system->push_event(Event(window_id(), EventType::WindowFocusGained, WindowFocusGainedEvent()));
                    break;

                case APP_CMD_LOST_FOCUS:
                    event_system->push_event(Event(window_id(), EventType::WindowFocusLost, WindowFocusLostEvent()));
                    break;

                case APP_CMD_LOW_MEMORY:
                    event_system->push_event(Event(window_id(), EventType::AppLowMemory, AppLowMemoryEvent()));
                    break;

                case APP_CMD_RESUME:
                    event_system->push_event(Event(window_id(), EventType::AppResume, AppResumeEvent()));
                    break;

                case APP_CMD_PAUSE:
                    event_system->push_event(Event(window_id(), EventType::AppPause, AppPauseEvent()));
                    break;

                case APP_CMD_STOP:
                case APP_CMD_DESTROY:
                    event_system->push_event(Event(window_id(), EventType::Quit, QuitEvent()));
                    break;

                default:
                    break;
            }
        }
    }

    static int32_t handle_key_event(AInputEvent* input_event)
    {
        int32_t event_key_code = AKeyEvent_getKeyCode(input_event);
        int32_t event_action   = AKeyEvent_getAction(input_event);

        if (event_action == AKEY_EVENT_ACTION_MULTIPLE)
            return 0;

        auto it           = keys.find(event_key_code);
        Keyboard::Key key = it == keys.end() ? Keyboard::Key::Unknown : it->second;


        if (event_action == AKEY_EVENT_ACTION_DOWN)
        {
            KeyDownEvent event;
            event.key = key;
            EventSystem::instance()->push_event(Event(window_id(), EventType::KeyDown, event));
        }
        else
        {
            KeyUpEvent event;
            event.key = key;
            EventSystem::instance()->push_event(Event(window_id(), EventType::KeyUp, event));
        }

        return 1;
    }

    template<typename EventDataType>
    static int32_t handle_mouse_button_event(AInputEvent* input_event, EventType type, int32_t event_pointer_index)
    {
        static int32_t pressed_buttons = 0;
        int_t buttons                  = AMotionEvent_getButtonState(input_event);

        int_t button    = pressed_buttons ^ buttons;
        pressed_buttons = buttons;


        EventDataType event_data;

        switch (button)
        {
            case AMOTION_EVENT_BUTTON_PRIMARY:
                event_data.button = Mouse::Button::Left;
                break;
            case AMOTION_EVENT_BUTTON_SECONDARY:
                event_data.button = Mouse::Button::Right;
                break;
            case AMOTION_EVENT_BUTTON_TERTIARY:
                event_data.button = Mouse::Button::Middle;
                break;
            case AMOTION_EVENT_BUTTON_BACK:
                event_data.button = Mouse::Button::Back;
                break;
            case AMOTION_EVENT_BUTTON_FORWARD:
                event_data.button = Mouse::Button::Forward;
                break;
        }

        float h      = Engine::WindowManager::instance()->main_window()->size().y;
        event_data.x = AMotionEvent_getX(input_event, event_pointer_index);
        event_data.y = h - AMotionEvent_getY(input_event, event_pointer_index);

        EventSystem::instance()->push_event(Event(window_id(), type, event_data));
        return 1;
    }

    static int32_t handle_mouse_event(AInputEvent* input_event, int32_t action, int32_t event_pointer_index)
    {
        int32_t result = 0;

        if (action == AMOTION_EVENT_ACTION_MOVE || action == AMOTION_EVENT_ACTION_HOVER_MOVE)
        {
            static MouseMotionEvent event;

            float h = Engine::WindowManager::instance()->main_window()->size().y;
            float x = AMotionEvent_getX(input_event, event_pointer_index);
            float y = h - AMotionEvent_getY(input_event, event_pointer_index);

            if (!Platform::m_android_platform_info.mouse_in_relative_mode)
            {
                event.xrel = x - event.x;
                event.yrel = y - event.y;
            }
            else
            {
                event.xrel = AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_RELATIVE_X, event_pointer_index);
                event.yrel = -AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_RELATIVE_Y, event_pointer_index);
            }

            event.x = x;
            event.y = y;
            EventSystem::instance()->push_event(Event(window_id(), EventType::MouseMotion, event));
            result = 1;
        }
        else if (action == AMOTION_EVENT_ACTION_SCROLL)
        {
            MouseWheelEvent event;
            event.x = AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_HSCROLL, event_pointer_index);
            event.y = AMotionEvent_getAxisValue(input_event, AMOTION_EVENT_AXIS_VSCROLL, event_pointer_index);

            if (glm::epsilonNotEqual(event.x, 0.f, 0.01f) || glm::epsilonNotEqual(event.y, 0.f, 0.01f))
            {
                EventSystem::instance()->push_event(Event(window_id(), EventType::MouseWheel, event));
                result = 1;
            }
        }
        else if (action == AMOTION_EVENT_ACTION_BUTTON_PRESS)
        {
            result =
                    handle_mouse_button_event<MouseButtonDownEvent>(input_event, EventType::MouseButtonDown, event_pointer_index);
        }
        else if (action == AMOTION_EVENT_ACTION_BUTTON_RELEASE)
        {
            result = handle_mouse_button_event<MouseButtonUpEvent>(input_event, EventType::MouseButtonUp, event_pointer_index);
        }

        return result;
    }

    static int32_t handle_finger_event(AInputEvent* input_event, int32_t action, int32_t event_pointer_index)
    {
        int32_t result = 0;

        if (action == AMOTION_EVENT_ACTION_MOVE)
        {
            static FingerMotionEvent event;

            for (int32_t i = 0, count = AMotionEvent_getPointerCount(input_event); i < count; ++i)
            {
                event.finger_index = static_cast<Index>(i);
                auto window        = Engine::WindowManager::instance()->main_window();
                float h            = window->size().y;
                event.x            = AMotionEvent_getX(input_event, i);
                event.y            = h - AMotionEvent_getY(input_event, i);

                Size2D prev_pos = TouchScreenSystem::instance()->finger_location(event.finger_index, window);
                event.xrel      = event.x - prev_pos.x;
                event.yrel      = event.y - prev_pos.y;

                if (glm::epsilonNotEqual(event.xrel, 0.f, 0.001f) || glm::epsilonNotEqual(event.yrel, 0.f, 0.001f))
                {
                    EventSystem::instance()->push_event(Event(window_id(), EventType::FingerMotion, event));
                }
            }

            result = 1;
        }
        else if (action == AMOTION_EVENT_ACTION_DOWN)
        {
            static FingerDownEvent event;

            if (event_pointer_index >= 0)
            {
                event.finger_index = static_cast<Index>(event_pointer_index);
                auto window        = Engine::WindowManager::instance()->main_window();
                float h            = window->size().y;
                event.x            = AMotionEvent_getX(input_event, event_pointer_index);
                event.y            = h - AMotionEvent_getY(input_event, event_pointer_index);

                EventSystem::instance()->push_event(Event(window_id(), EventType::FingerDown, event));
            }

            result = 1;
        }
        else if (action == AMOTION_EVENT_ACTION_UP)
        {
            static FingerUpEvent event;

            if (event_pointer_index >= 0)
            {
                event.finger_index = static_cast<Index>(event_pointer_index);
                EventSystem::instance()->push_event(Event(window_id(), EventType::FingerUp, event));
            }

            result = 1;
        }
        return result;
    }

    static int32_t handle_motion_event(AInputEvent* input_event)
    {
        int32_t action = AMotionEvent_getAction(input_event);
        int32_t event_pointer_index =
                (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        action &= AMOTION_EVENT_ACTION_MASK;

        switch (AMotionEvent_getToolType(input_event, event_pointer_index))
        {
            case AMOTION_EVENT_TOOL_TYPE_MOUSE:
                return handle_mouse_event(input_event, action, event_pointer_index);
            case AMOTION_EVENT_TOOL_TYPE_FINGER:
                return handle_finger_event(input_event, action, event_pointer_index);
            case AMOTION_EVENT_TOOL_TYPE_STYLUS:
            case AMOTION_EVENT_TOOL_TYPE_ERASER:
            case AMOTION_EVENT_TOOL_TYPE_PALM:
            default:
                break;
        }

        return 0;
    }

    static int32_t handle_input_event(struct android_app* app, AInputEvent* input_event)
    {
        int32_t result = 0;

        int32_t event_type = AInputEvent_getType(input_event);

        switch (event_type)
        {
            case AINPUT_EVENT_TYPE_KEY:
                result = handle_key_event(input_event);
                break;
            case AINPUT_EVENT_TYPE_MOTION:
                result = handle_motion_event(input_event);
                break;
        }

        if (m_window)
        {
            result = glm::max(m_window->process_imgui_event(input_event), result);
        }

        return 1;
    }

    void initialize_android_application(struct android_app* app)
    {
        m_application = app;

        app->onAppCmd     = handle_app_cmd;
        app->onInputEvent = handle_input_event;

        // Wait activity initialization
        while (m_is_inited == false)
        {
            WindowManager::wait_for_events();
        }
    }

    android_app* android_application()
    {
        return m_application;
    }

    namespace WindowManager
    {
        ENGINE_EXPORT void initialize()
        {
            // Nothing ?
        }

        ENGINE_EXPORT void terminate()
        {
            // Nothing ?
        }

        ENGINE_EXPORT Window* create_window(const WindowConfig* config)
        {
            if (m_window)
            {
                throw EngineException("Cannot create two windows on android!");
            }

            if (rhi->info.struct_instance->base_name() == "VULKAN")
            {
                m_window = new AndroidVulkanWindow(config);
            }
            else
            {
                m_window = new AndroidEGLWindow(config);
            }
            return m_window;
        }

        ENGINE_EXPORT void destroy_window(Window* window)
        {
            m_window = nullptr;
            delete window;
        }

        ENGINE_EXPORT void mouse_relative_mode(bool flag)
        {
            Platform::m_android_platform_info.mouse_in_relative_mode = flag;
            Engine::WindowManager::instance()->main_window()->cursor_mode(flag ? CursorMode::Hidden : CursorMode::Normal);
        }

        static void execute_pool_source(struct android_poll_source* source)
        {
            if (source != nullptr)
                source->process(m_application, source);

            if (m_application->destroyRequested != 0)
            {
                EventSystem::instance()->push_event(Event(0, EventType::Quit, QuitEvent()));
            }
        }

        static void android_get_events(int timeout)
        {
            android_poll_source* source;
            auto result = ALooper_pollOnce(timeout, nullptr, nullptr, (void**) &source);

            if (result == ALOOPER_POLL_ERROR)
            {
                throw EngineException("Android: ALOOPER_POLL_ERROR");
            }

            execute_pool_source(source);
        }

        ENGINE_EXPORT void pool_events()
        {
            android_get_events(0);
        }

        ENGINE_EXPORT void wait_for_events()
        {
            android_get_events(-1);
        }
    }// namespace WindowManager
}// namespace Engine::Platform
