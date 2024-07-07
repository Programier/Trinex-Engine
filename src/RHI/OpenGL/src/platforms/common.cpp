#include <SDL.h>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
    void* create_opengl_context(Window* main_window)
    {
        SDL_Window* window = reinterpret_cast<SDL_Window*>(main_window->native_window());
        void* context      = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, context);
        return context;
    }

    void make_window_current(Window* window, void* context)
    {
        SDL_Window* sdl_window = reinterpret_cast<SDL_Window*>(window->native_window());
        SDL_GL_MakeCurrent(sdl_window, context);
    }

    bool has_window_vsync(Window* window, void*)
    {
        return SDL_GL_GetSwapInterval();
    }

    void set_window_vsync(Window* window, void* context, bool flag)
    {
        SDL_GL_SetSwapInterval(flag ? 1 : 0);
    }

    void swap_window_buffers(Window* window, void* context)
    {
        SDL_Window* sdl_window = reinterpret_cast<SDL_Window*>(window->native_window());
        SDL_GL_SwapWindow(sdl_window);
    }

    void destroy_opengl_context(void* context)
    {
        SDL_GL_DeleteContext(context);
    }
}// namespace Engine
