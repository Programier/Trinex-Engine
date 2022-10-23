#include <SDL.h>
#include <iostream>
#include <sdl_surface.hpp>

SDL_Surface* Engine::create_sdl_surface(const Image& image)
{
    if (image.empty())
        return nullptr;


    // Calculate pitch
    int pitch, channels;
    pitch = image.width() * (channels = image.channels());
    pitch = (pitch + 3) & ~3;

    // Setup relevance bitmask
    int Rmask, Gmask, Bmask, Amask;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    Rmask = 0x000000FF;
    Gmask = 0x0000FF00;
    Bmask = 0x00FF0000;
    Amask = (channels == 4) ? 0xFF000000 : 0;
#else
    int s = (channels == 4) ? 0 : 8;
    Rmask = 0xFF000000 >> s;
    Gmask = 0x00FF0000 >> s;
    Bmask = 0x0000FF00 >> s;
    Amask = 0x000000FF >> s;
#endif
    return SDL_CreateRGBSurfaceFrom((void*) image.data(), image.width(), image.height(), channels * 8, pitch, Rmask, Gmask,
                                    Bmask, Amask);
}
