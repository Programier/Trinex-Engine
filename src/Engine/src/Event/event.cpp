#include <Event/keyboard_event.hpp>
#include <Event/update_events.hpp>
#include <SDL.h>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <private_event.hpp>

// Internal event system functions

static std::size_t _M_time = 0;
static std::size_t _M_diff_time = 0;
static std::size_t _M_last_time = 0;


namespace Engine
{
    Event event;
    ENGINE_EXPORT std::vector<void (*)(void*)> Event::sdl_callbacks;
    ENGINE_EXPORT std::vector<void (*)(unsigned int)> Event::on_sensor_update;
    ENGINE_EXPORT std::vector<void (*)()> Event::on_quit;
    ENGINE_EXPORT std::vector<void (*)()> Event::on_terminate;
    ENGINE_EXPORT std::vector<void (*)()> Event::on_resume;
    ENGINE_EXPORT std::vector<void (*)()> Event::on_pause;
    ENGINE_EXPORT std::vector<void (*)()> Event::on_low_memory;

    void process_keyboard_event(SDL_KeyboardEvent& event);
    void clear_keyboard_events();

    void process_window_event(SDL_WindowEvent& event);

    void clear_mouse_event();
    void mouse_process_event(SDL_MouseMotionEvent& event);
    void mouse_process_event(SDL_MouseButtonEvent& event);
    void mouse_process_event(SDL_MouseWheelEvent& event);
    void process_paths_event(SDL_DropEvent& event);
    void process_text_event(SDL_TextInputEvent& event);
    void process_text_event(SDL_TextEditingEvent& event);
    void process_text_event(SDL_TextEditingExtEvent& event);

    void clear_touchscreen_events();
    void process_touchscreen_event(SDL_TouchFingerEvent& event);

    ENGINE_EXPORT const Event& Event::poll_events()
    {
        UpdateEvent::poll_events();
        return event;
    }

    ENGINE_EXPORT const Event& Event::wait_for_event()
    {
        UpdateEvent::wait_for_event();
        return event;
    }

    ENGINE_EXPORT std::size_t Event::time()
    {
        return _M_time;
    }

    ENGINE_EXPORT std::size_t Event::diff_time()
    {
        return _M_diff_time;
    }

    ENGINE_EXPORT const Event& get_event()
    {
        return event;
    }
}// namespace Engine

// End of Internal event system functions

namespace Engine::UpdateEvent
{
    static SDL_Event event;

    static void clear_event_system()
    {
        _M_time = SDL_GetTicks64();
        _M_diff_time = _M_time - _M_last_time;
        _M_last_time = _M_time;

        clear_keyboard_events();
        clear_mouse_event();
        clear_touchscreen_events();
    }


    template<typename Container, typename... Args>
    void run_callbacks(const Container& container, const Args... args)
    {
        for (auto func : container) func(args...);
    }

    //    /* Audio hotplug events */
    //    SDL_AUDIODEVICEADDED = 0x1100, /**< A new audio device is available */
    //    SDL_AUDIODEVICEREMOVED,        /**< An audio device has been removed. */


    //SDL_JoyAxisEvent jaxis; /**< Joystick axis event data */
    //SDL_JoyBallEvent jball;                 /**< Joystick ball event data */
    //SDL_JoyHatEvent jhat;                   /**< Joystick hat event data */
    //SDL_JoyButtonEvent jbutton;             /**< Joystick button event data */
    //SDL_JoyDeviceEvent jdevice;             /**< Joystick device change event data */
    //SDL_JoyBatteryEvent jbattery;           /**< Joystick battery event data */
    //SDL_ControllerAxisEvent caxis;          /**< Game Controller axis event data */
    //SDL_ControllerButtonEvent cbutton;      /**< Game Controller button event data */
    //SDL_ControllerDeviceEvent cdevice;      /**< Game Controller device event data */
    //SDL_ControllerTouchpadEvent ctouchpad;  /**< Game Controller touchpad event data */
    //SDL_ControllerSensorEvent csensor;      /**< Game Controller sensor event data */
    //SDL_AudioDeviceEvent adevice; /**< Audio device event data */

    static void process_event()
    {
        switch (event.type)
        {
            case SDL_APP_LOWMEMORY:
                run_callbacks(Event::on_low_memory);
                break;

            case SDL_APP_WILLENTERBACKGROUND:
            case SDL_APP_DIDENTERBACKGROUND:
                run_callbacks(Event::on_pause);
                break;

            case SDL_APP_TERMINATING:
                run_callbacks(Event::on_terminate);
                break;

            case SDL_APP_DIDENTERFOREGROUND:
            case SDL_APP_WILLENTERFOREGROUND:
                run_callbacks(Event::on_resume);
                break;

            case SDL_QUIT:
                Window::close();
                run_callbacks(Event::on_quit);
                break;

            case SDL_TEXTINPUT:
                process_text_event(event.text);
                break;

            case SDL_TEXTEDITING:
                process_text_event(event.edit);
                break;

            case SDL_TEXTEDITING_EXT:
                process_text_event(event.editExt);
                break;

            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_LEAVE)
                    on_leave();
                else
                    process_window_event(event.window);
                break;
            }

            case SDL_MOUSEMOTION:
            {
                mouse_process_event(event.motion);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                mouse_process_event(event.button);
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                mouse_process_event(event.wheel);
                break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                process_keyboard_event(event.key);
                break;
            }

            case SDL_DROPFILE:
            case SDL_DROPCOMPLETE:
            case SDL_DROPTEXT:
            case SDL_DROPBEGIN:
                process_paths_event(event.drop);
                break;

            case SDL_DISPLAYEVENT_CONNECTED:
                Monitor::update();
                break;

            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
                process_touchscreen_event(event.tfinger);
                break;

            case SDL_SENSORUPDATE:
                run_callbacks(Event::on_sensor_update, static_cast<unsigned int>(event.sensor.which));
                break;

            default:
                break;
        }

        run_callbacks(Event::sdl_callbacks, &event);
    }


    ENGINE_EXPORT void poll_events()
    {
        clear_event_system();
        while (SDL_PollEvent(&event)) process_event();
    }

    ENGINE_EXPORT void wait_for_event()
    {
        clear_event_system();
        SDL_WaitEvent(&event);
        process_event();
    }
}// namespace Engine::UpdateEvent