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
	};
}// namespace Trinex::Platform
