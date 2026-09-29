#pragma once
#include <CommonPlatform/context.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT SDLContext : public CommonContext
	{
	public:
		SDLContext();
		~SDLContext();

		Clipboard* clipboard() override;
		LibraryLoader* library_loader() override;
		ProcessSystem* process_system() override;
		ThreadSystem* thread_system() override;
		MonitorSystem* monitor_system() override;
		DialogSystem* dialog_system() override;
		WindowSystem* window_system() override;
	};
}// namespace Trinex::Platform
