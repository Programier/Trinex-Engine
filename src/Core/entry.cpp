#include <Core/entry.hpp>
#include <Core/reflection/deprecated/class.hpp>

namespace Trinex
{
	i32 EntryPoint::execute()
	{
		return 0;
	}

	trinex_implement_engine_class_default_init(EntryPoint, 0);
} // namespace Trinex
