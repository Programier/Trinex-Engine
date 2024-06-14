#include <SDL2/SDL.h>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
    void* create_opengl_context()
    {
        SDL_Window* window = reinterpret_cast<SDL_Window*>(WindowManager::instance()->main_window()->native_window());
        void* context      = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, context);
        return context;
    }

    void make_window_current(void* native_window, void* context)
    {
        SDL_Window* window = reinterpret_cast<SDL_Window*>(native_window);
        SDL_GL_MakeCurrent(window, context);
    }

    bool has_window_vsync(void*, void*)
    {
        return SDL_GL_GetSwapInterval();
    }

    void set_window_vsync(void* native_window, void* context, bool flag)
    {
        SDL_GL_SetSwapInterval(flag ? 1 : 0);
    }

    void swap_window_buffers(void* native_window, void* context)
    {
        SDL_Window* window = reinterpret_cast<SDL_Window*>(native_window);
        SDL_GL_SwapWindow(window);
    }

    void destroy_opengl_context(void* context)
    {
        SDL_GL_DeleteContext(context);
    }
}// namespace Engine
