#include <Core/engine.hpp>
#include <Entry/engine.hpp>

namespace Trinex
{
	Ref<Engine> EngineEntry::create_engine()
	{
		return Ref<Engine>::make();
	}

	i32 EngineEntry::execute()
	{
		Ref<Engine> engine = create_engine();
		return engine->run();
	}
} // namespace Trinex
