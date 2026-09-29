#include <SDL3/SDL_init.h>
#include <SDLPlatform/clipboard.hpp>
#include <SDLPlatform/context.hpp>
#include <SDLPlatform/dialog.hpp>
#include <SDLPlatform/library.hpp>
#include <SDLPlatform/monitor.hpp>
#include <SDLPlatform/process.hpp>
#include <SDLPlatform/threading.hpp>
#include <SDLPlatform/window.hpp>

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

	MonitorSystem* SDLContext::monitor_system()
	{
		return SDLMonitorSystem::instance();
	}

	DialogSystem* SDLContext::dialog_system()
	{
		return SDLDialogSystem::instance();
	}

	WindowSystem* SDLContext::window_system()
	{
		return SDLWindowSystem::instance();
	}
}// namespace Trinex::Platform
