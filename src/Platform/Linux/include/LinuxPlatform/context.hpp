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
	};
}// namespace Trinex::Platform
