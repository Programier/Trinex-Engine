#pragma once
#include <SDLPlatform/context.hpp>

namespace Trinex::Platform
{
	class trinex_api LinuxContext : public SDLContext
	{
	public:
		~LinuxContext() override;

		System* system() override;
		Memory* memory() override;
	};
}// namespace Trinex::Platform
