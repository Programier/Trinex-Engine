#include <SDL3/SDL_init.h>
#include <SDLPlatform/clipboard.hpp>
#include <SDLPlatform/context.hpp>
#include <SDLPlatform/library.hpp>

namespace Trinex::Platform
{
	SDLContext::SDLContext()
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	SDLContext::~SDLContext()
	{
		SDL_Quit();
	}

	Clipboard* SDLContext::clipboard()
	{
		return SDLClipboard::instance();
	}

	LibraryLoader* SDLContext::library_loader()
	{
		return SDLLibraryLoader::instance();
	}
}// namespace Trinex::Platform
