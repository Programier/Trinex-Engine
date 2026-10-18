#pragma once
#include <Platform/library.hpp>

struct SDL_SharedObject;

namespace Trinex::Platform
{
	class trinex_api SDLLibrary : public Library
	{
	private:
		SDL_SharedObject* m_handle = nullptr;

	public:
		SDLLibrary(SDL_SharedObject* handle);
		~SDLLibrary();

		LibrarySymbol find(const char* name) override;
	};

	class trinex_api SDLLibraryLoader : public LibraryLoader
	{
	public:
		static SDLLibraryLoader* instance();

		Ref<Library> load(const char* path) override;
	};
}// namespace Trinex::Platform
