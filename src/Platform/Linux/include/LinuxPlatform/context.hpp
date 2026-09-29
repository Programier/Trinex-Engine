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
		WindowSystem* window_system() override;
		InputSystem* input_system() override;
		EventLoop* event_loop() override;
	};
}// namespace Trinex::Platform
