#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDLPlatform/clipboard.hpp>
#include <SDLPlatform/context.hpp>
#include <SDLPlatform/dialog.hpp>
#include <SDLPlatform/input.hpp>
#include <SDLPlatform/library.hpp>
#include <SDLPlatform/monitor.hpp>
#include <SDLPlatform/process.hpp>
#include <SDLPlatform/threading.hpp>
#include <SDLPlatform/window.hpp>

namespace Trinex::Platform
{
	static void SDLCALL sdl_log(void*, int, SDL_LogPriority, const char* message)
	{
		trinex_info(Log::Platform, "SDL: {}", message);
	}

	SDLContext::SDLContext()
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS);
		SDL_SetLogOutputFunction(sdl_log, nullptr);
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

	InputSystem* SDLContext::input_system()
	{
		return SDLInputSystem::instance();
	}

	void SDLContext::update()
	{
		SDLInputSystem::instance()->update();
	}
} // namespace Trinex::Platform
