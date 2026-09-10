#pragma once
#include <SDLPlatform/context.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT LinuxContext : public SDLContext
	{
	public:
		~LinuxContext() override;

		System* system() override;
		Memory* memory() override;
		DisplaySystem* display_system() override;
		WindowSystem* window_system() override;
		InputSystem* input_system() override;
		EventLoop* event_loop() override;
		FileSystem* filesystem() override;
		FileWatcher* file_watcher() override;
		ThreadSystem* thread_system() override;
		DialogSystem* dialogs() override;
	};
}// namespace Trinex::Platform
