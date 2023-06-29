#include <Event/keyboard_event.hpp>
#include <Event/update_events.hpp>
#include <SDL.h>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <chrono>
#include <private_event.hpp>

// Internal event system functions

#define CURRENT_TIME std::chrono::steady_clock::now()
static double _M_diff_time         = -1.f;
static auto _M_prev_time           = CURRENT_TIME;
static auto _M_start_time          = CURRENT_TIME;
static std::size_t _M_frame_number = 0;


namespace Engine
{
    static Event event;
    ENGINE_EXPORT Set<void (*)(void*)> Event::sdl_callbacks;
    ENGINE_EXPORT Set<void (*)(unsigned int)> Event::on_sensor_update;
    ENGINE_EXPORT Set<void (*)()> Event::on_quit;
    ENGINE_EXPORT Set<void (*)()> Event::on_terminate;
    ENGINE_EXPORT Set<void (*)()> Event::on_resume;
    ENGINE_EXPORT Set<void (*)()> Event::on_pause;
    ENGINE_EXPORT Set<void (*)()> Event::on_low_memory;

    void process_keyboard_event(SDL_KeyboardEvent& event);
    void clear_keyboard_events();

    void clear_mouse_event();
    void mouse_process_event(SDL_MouseMotionEvent& event);
    void mouse_process_event(SDL_MouseButtonEvent& event);
    void mouse_process_event(SDL_MouseWheelEvent& event);
    void process_text_event(SDL_TextInputEvent& event);
    void process_text_event(SDL_TextEditingEvent& event);
    void process_text_event(SDL_TextEditingExtEvent& event);

    void clear_touchscreen_events();
    void process_touchscreen_event(SDL_TouchFingerEvent& event);

    ENGINE_EXPORT const Event& Event::poll_events()
    {
        UpdateEvents::poll_events();
        return event;
    }

    ENGINE_EXPORT const Event& Event::wait_for_event()
    {
        UpdateEvents::wait_for_event();
        return event;
    }


    ENGINE_EXPORT double Event::diff_time()
    {
        return _M_diff_time;
    }

    ENGINE_EXPORT double Event::time()
    {
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(CURRENT_TIME - _M_start_time).count();
        return static_cast<double>(time) / 1000000.0;
    }

    ENGINE_EXPORT const Event& get_event()
    {
        return event;
    }

    ENGINE_EXPORT size_t Event::frame_number()
    {
        return _M_frame_number;
    }


    static SDL_Event internal_event;

    static void clear_event_system()
    {
        auto current = std::chrono::steady_clock::now();
        auto diff    = std::chrono::duration_cast<std::chrono::microseconds>(current - _M_prev_time).count();
        _M_prev_time = current;
        _M_diff_time = static_cast<double>(diff) / 1000000.0;
        clear_keyboard_events();
        clear_mouse_event();
        clear_touchscreen_events();
        _M_frame_number++;
    }


    template<typename Container, typename... Args>
    static void run_callbacks(const Container& container, const Args... args)
    {
        for (auto func : container) func(args...);
    }

    void UpdateEvents::process_event()
    {
        switch (internal_event.type)
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
                Window::instance()->close();
                run_callbacks(Event::on_quit);
                break;

            case SDL_TEXTINPUT:
                process_text_event(internal_event.text);
                break;

            case SDL_TEXTEDITING:
                process_text_event(internal_event.edit);
                break;

            case SDL_TEXTEDITING_EXT:
                process_text_event(internal_event.editExt);
                break;

            case SDL_WINDOWEVENT:
            {
                if (internal_event.window.event == SDL_WINDOWEVENT_LEAVE)
                    on_leave();
                else
                    Window::instance()->process_window_event(internal_event.window);
                break;
            }

            case SDL_MOUSEMOTION:
            {
                mouse_process_event(internal_event.motion);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                mouse_process_event(internal_event.button);
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                mouse_process_event(internal_event.wheel);
                break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                process_keyboard_event(internal_event.key);
                break;
            }

            case SDL_DROPFILE:
            case SDL_DROPCOMPLETE:
            case SDL_DROPTEXT:
            case SDL_DROPBEGIN:
                Window::instance()->process_paths_event(internal_event.drop);
                break;

            case SDL_DISPLAYEVENT_CONNECTED:
                Monitor::update();
                break;

            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
                process_touchscreen_event(internal_event.tfinger);
                break;

            case SDL_SENSORUPDATE:
                run_callbacks(Event::on_sensor_update, static_cast<unsigned int>(internal_event.sensor.which));
                break;

            default:
                break;
        }

        run_callbacks(Event::sdl_callbacks, &event);
    }


    void UpdateEvents::poll_events()
    {
        clear_event_system();
        while (SDL_PollEvent(&internal_event)) process_event();
    }

    void UpdateEvents::wait_for_event()
    {
        clear_event_system();
        SDL_WaitEvent(&internal_event);
        process_event();
        poll_events();
    }

}// namespace Engine


namespace Engine
{

}// namespace Engine
