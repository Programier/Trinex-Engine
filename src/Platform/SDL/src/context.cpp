#include <SDL3/SDL_init.h>
#include <SDLPlatform/clipboard.hpp>
#include <SDLPlatform/context.hpp>
#include <SDLPlatform/library.hpp>
#include <SDLPlatform/process.hpp>
#include <SDLPlatform/threading.hpp>

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

	ProcessSystem* SDLContext::process_system()
	{
		return SDLProcessSystem::instance();
	}

	ThreadSystem* SDLContext::thread_system()
	{
		return SDLThreadSystem::instance();
	}
}// namespace Trinex::Platform
