#pragma once
#include <Core/callback.hpp>

namespace Trinex
{
	class Object;

	namespace Refl
	{
		class Class;
	};

	class trinex_api GarbageCollector final
	{
	public:
		trinex_api static CallBacks<void(Object*)> on_unreachable_check;

		trinex_api static void destroy(Object* object);
		trinex_api static void update(float dt);

		friend class EngineLoop;
		friend class Class;
		friend class Refl::Class;

	private:
		trinex_api static void submit_current_stage();
		trinex_api static bool process_objects(void (*callback)(Object* object));
		trinex_api static void mark_unreachable(float dt);
		trinex_api static void collect_garbage(float dt);
		trinex_api static void destroy_garbage(float dt);

		trinex_api static void destroy_all_objects();
	};
}// namespace Trinex
