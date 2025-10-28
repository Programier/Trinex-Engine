#pragma once
#include <Core/task.hpp>
#include <Core/thread.hpp>

namespace Engine
{
	ENGINE_EXPORT void create_threads();
	ENGINE_EXPORT void destroy_threads();
	ENGINE_EXPORT Thread* logic_thread();
	ENGINE_EXPORT Thread* this_thread();

	inline bool is_in_logic_thread()
	{
		return this_thread() == logic_thread();
	}
}// namespace Engine
