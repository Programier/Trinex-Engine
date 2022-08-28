#pragma once
#include <Image/image.hpp>

class SDL_Surface;
namespace Engine
{
    SDL_Surface* create_sdl_surface(const Image& image);
}
