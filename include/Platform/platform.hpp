#pragma once
#include <Platform/clipboard.hpp>
#include <Platform/dialogs.hpp>
#include <Platform/events.hpp>
#include <Platform/input.hpp>
#include <Platform/library.hpp>
#include <Platform/memory.hpp>
#include <Platform/monitor.hpp>
#include <Platform/process.hpp>
#include <Platform/system.hpp>
#include <Platform/threading.hpp>
#include <Platform/window.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT Context
	{
	private:
		static Context* s_instance;

	public:
		static Context* instance();

		Context();
		virtual ~Context();

		virtual System* system()                = 0;
		virtual Memory* memory()                = 0;
		virtual MonitorSystem* monitor_system() = 0;
		virtual WindowSystem* window_system()   = 0;
		virtual InputSystem* input_system()     = 0;
		virtual EventLoop* event_loop()         = 0;
		virtual LibraryLoader* library_loader() = 0;
		virtual ProcessSystem* process_system() = 0;
		virtual ThreadSystem* thread_system()   = 0;
		virtual Clipboard* clipboard()          = 0;
		virtual DialogSystem* dialogs()         = 0;
	};
}// namespace Trinex::Platform
