#include <Platform/platform.hpp>

namespace Trinex::Platform
{
	LibraryLoader::~LibraryLoader() {}

	Context::Context() {}

	Context::~Context() {}

	System* System::instance()
	{
		return Context::instance()->system();
	}

	Memory* Memory::instance()
	{
		return Context::instance()->memory();
	}

	DisplaySystem* DisplaySystem::instance()
	{
		return Context::instance()->display_system();
	}

	WindowSystem* WindowSystem::instance()
	{
		return Context::instance()->window_system();
	}

	InputSystem* InputSystem::instance()
	{
		return Context::instance()->input_system();
	}

	EventLoop* EventLoop::instance()
	{
		return Context::instance()->event_loop();
	}

	FileSystem* FileSystem::instance()
	{
		return Context::instance()->filesystem();
	}

	FileWatcher* FileWatcher::instance()
	{
		return Context::instance()->file_watcher();
	}

	LibraryLoader* LibraryLoader::instance()
	{
		return Context::instance()->library_loader();
	}

	ProcessSystem* ProcessSystem::instance()
	{
		return Context::instance()->process_system();
	}

	ThreadSystem* ThreadSystem::instance()
	{
		return Context::instance()->thread_system();
	}

	Clipboard* Clipboard::instance()
	{
		return Context::instance()->clipboard();
	}

	DialogSystem* DialogSystem::instance()
	{
		return Context::instance()->dialogs();
	}
}// namespace Trinex::Platform
