#pragma once
#include <Image/image.hpp>

struct SDL_Surface;
namespace Engine
{
    SDL_Surface* create_sdl_surface(const Image& image);
}
