#include <LinuxPlatform/context.hpp>
#include <LinuxPlatform/memory.hpp>
#include <LinuxPlatform/system.hpp>

namespace Trinex::Platform
{
	LinuxContext::~LinuxContext() = default;

	Context* Context::instance()
	{
		static LinuxContext ctx;
		return &ctx;
	}

	System* LinuxContext::system()
	{
		return LinuxSystem::instance();
	}

	Memory* LinuxContext::memory()
	{
		return LinuxMemory::instance();
	}

	DisplaySystem* LinuxContext::display_system()
	{
		return nullptr;
	}

	WindowSystem* LinuxContext::window_system()
	{
		return nullptr;
	}

	InputSystem* LinuxContext::input_system()
	{
		return nullptr;
	}

	EventLoop* LinuxContext::event_loop()
	{
		return nullptr;
	}

	FileSystem* LinuxContext::filesystem()
	{
		return nullptr;
	}

	FileWatcher* LinuxContext::file_watcher()
	{
		return nullptr;
	}

	ThreadSystem* LinuxContext::thread_system()
	{
		return nullptr;
	}

	DialogSystem* LinuxContext::dialogs()
	{
		return nullptr;
	}
}// namespace Trinex::Platform
