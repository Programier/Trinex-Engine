#pragma once
#include <Core/export.hpp>
#include <Core/callback.hpp>

namespace Engine
{
    class Object;
    struct ENGINE_EXPORT GarbageCollector final {
        ENGINE_EXPORT static CallBacks<void(Object*)> on_unreachable_check;
        ENGINE_EXPORT static CallBacks<void(Object*)> on_destroy;

        ENGINE_EXPORT static void destroy(Object* object);
        ENGINE_EXPORT static void update(float dt);

        friend class EngineLoop;

    private:
        ENGINE_EXPORT static void submit_current_stage();
        ENGINE_EXPORT static bool process_objects(void (*callback)(Object* object));
        ENGINE_EXPORT static void mark_unreachable(float dt);
        ENGINE_EXPORT static void collect_garbage(float dt);
        ENGINE_EXPORT static void destroy_garbage(float dt);
        ENGINE_EXPORT static void destroy_internal(Object* object, bool destroy_owner_if_exist = false);

        ENGINE_EXPORT static void destroy_all_objects();
    };
}// namespace Engine
