#include <SDL.h>
#include <stdexcept>

namespace Engine
{
    void trinex_init_sdl()
    {
        if (SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO))
            throw std::runtime_error(SDL_GetError());
    }

    void trinex_terminate_sdl()
    {
        SDL_Quit();
    }
}// namespace Engine
