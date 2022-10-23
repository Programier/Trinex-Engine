#include <Event/push_event.hpp>
#include <SDL_events.h>

namespace Engine::PushEvent
{
    static SDL_Event event;

    static void push_event(SDL_EventType type)
    {
        event.type = type;
        SDL_PushEvent(&event);
    }

    ENGINE_EXPORT void on_terminate_event()
    {
        push_event(SDL_APP_TERMINATING);
    }


    ENGINE_EXPORT void on_quit_event()
    {
        push_event(SDL_QUIT);
    }

    ENGINE_EXPORT void on_resume_event()
    {
        push_event(SDL_APP_DIDENTERFOREGROUND);
    }

    ENGINE_EXPORT void on_pause_event()
    {
        push_event(SDL_APP_DIDENTERBACKGROUND);
    }

    ENGINE_EXPORT void on_low_memory()
    {
        push_event(SDL_APP_LOWMEMORY);
    }
}// namespace Engine::PushEvent
