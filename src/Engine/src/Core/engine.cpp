#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/entry_point.hpp>
#include <Core/executable_object.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Engine/world.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <Window/window_manager.hpp>
#include <no_api.hpp>

namespace Engine
{
    implement_engine_class_default_init(EngineInstance, 0);

    ENGINE_EXPORT const String& EngineInstance::project_name()
    {
        static String name = "Trinex Engine";
        return name;
    }

    ENGINE_EXPORT const String& EngineInstance::project_name(const String& name)
    {
        const_cast<String&>(project_name()) = name;
        return project_name();
    }

    /////////////////// INITIALIZE ENGINE ///////////////////


    ENGINE_EXPORT int_t EngineInstance::init()
    {
        return Super::init();
    }

    ENGINE_EXPORT int_t EngineInstance::terminate()
    {
        return Super::terminate();
    }


}// namespace Engine
