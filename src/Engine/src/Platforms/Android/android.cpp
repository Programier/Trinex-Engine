#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/definitions.hpp>
#include <Core/string_functions.hpp>
#include <Platform/platform.hpp>

namespace Engine::Platform
{
    ENGINE_EXPORT OperationSystemType system_type()
    {
        return OperationSystemType::Android;
    }

    ENGINE_EXPORT const char* system_name()
    {
        return "Android";
    }

    ENGINE_EXPORT Path find_root_directory(int_t argc, const char** argv)
    {
        // TODO: It needs to be made more scalable
        return Strings::format("/sdcard/TrinexGames/{}/", ConfigManager::get_string("Engine::project_name"));
    }

    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives()
    {
        return {{"/", "/"}};
    }
}// namespace Engine::Platform
