#pragma once
#include <Platform/clipboard.hpp>
#include <Platform/dialogs.hpp>
#include <Platform/display.hpp>
#include <Platform/events.hpp>
#include <Platform/file_watcher.hpp>
#include <Platform/filesystem.hpp>
#include <Platform/input.hpp>
#include <Platform/library.hpp>
#include <Platform/memory.hpp>
#include <Platform/process.hpp>
#include <Platform/resource_ptr.hpp>
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
		virtual DisplaySystem* display_system() = 0;
		virtual WindowSystem* window_system()   = 0;
		virtual InputSystem* input_system()     = 0;
		virtual EventLoop* event_loop()         = 0;
		virtual FileSystem* filesystem()        = 0;
		virtual FileWatcher* file_watcher()     = 0;
		virtual LibraryLoader* library_loader() = 0;
		virtual ProcessSystem* process_system() = 0;
		virtual ThreadSystem* thread_system()   = 0;
		virtual Clipboard* clipboard()          = 0;
		virtual DialogSystem* dialogs()         = 0;

		FORCE_INLINE SystemType system_type() const { return const_cast<Context*>(this)->system()->system_type(); }
		FORCE_INLINE const String* name() const { return const_cast<Context*>(this)->system()->name(); }
	};
}// namespace Trinex::Platform
