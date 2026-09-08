#include <SDL3/SDL_loadso.h>
#include <SDLPlatform/library.hpp>

namespace Trinex::Platform
{
	SDLLibrary::SDLLibrary(SDL_SharedObject* handle) : m_handle(handle) {}

	SDLLibrary::~SDLLibrary()
	{
		if (m_handle)
		{
			SDL_UnloadObject(m_handle);
		}
	}

	LibrarySymbol SDLLibrary::find(const char* name)
	{
		return {.address = SDL_LoadFunction(m_handle, name)};
	}

	SDLLibraryLoader* SDLLibraryLoader::instance()
	{
		static SDLLibraryLoader loader;
		return &loader;
	}

	SDLLibrary* SDLLibraryLoader::load(const char* path)
	{
		if (auto handle = SDL_LoadObject(path))
		{
			return trx_new SDLLibrary(handle);
		}

		return nullptr;
	}

	SDLLibraryLoader& SDLLibraryLoader::unload(Library* library)
	{
		if (library)
		{
			trx_delete library;
		}
		return *this;
	}
}// namespace Trinex::Platform
