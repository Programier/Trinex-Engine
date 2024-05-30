#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/definitions.hpp>
#include <Core/string_functions.hpp>
#include <android_platform.hpp>
#include <android_native_app_glue.h>

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
        String path = Strings::format("/sdcard/TrinexGames/{}/", ConfigManager::get_string("Engine::project_name"));
        return path;
    }

    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives()
    {
        return {{"/", "/"}};
    }
}// namespace Engine::Platform
