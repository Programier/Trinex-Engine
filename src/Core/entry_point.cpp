#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/entry_point.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>

namespace Engine
{
	int_t EntryPoint::execute()
	{
		return 0;
	}

	trinex_implement_engine_class_default_init(EntryPoint, 0);
}// namespace Engine
