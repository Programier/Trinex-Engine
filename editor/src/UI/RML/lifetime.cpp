#include <RmlUi/Core.h>

namespace Trinex
{
	trinex_on_pre_init()
	{
		trinex_verify(Rml::Initialise());
		Rml::CreateContext("Trinex RML", {1280, 720});
	}

	trinex_on_post_shutdown()
	{
		Rml::Shutdown();
	}
}// namespace Trinex
