#pragma once
#include <Platform/library.hpp>

struct SDL_SharedObject;

namespace Trinex::Platform
{
	class ENGINE_EXPORT SDLLibrary : public Library
	{
	private:
		SDL_SharedObject* m_handle = nullptr;

	public:
		SDLLibrary(SDL_SharedObject* handle);
		~SDLLibrary();

		LibrarySymbol find(const char* name) override;
	};

	class ENGINE_EXPORT SDLLibraryLoader : public LibraryLoader
	{
	public:
		static SDLLibraryLoader* instance();

		SDLLibrary* load(const char* path) override;
		SDLLibraryLoader& unload(Library* library) override;
	};
}// namespace Trinex::Platform
