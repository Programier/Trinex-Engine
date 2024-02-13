#include <Core/definitions.hpp>
#include <Core/platform.hpp>

#if PLATFORM_ANDROID
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

    ENGINE_EXPORT Path find_root_directory(int_t argc, char** argv)
    {
        // TODO: It needs to be made more scalable
        return Strings::format("/sdcard/TrinexGames/{}/", EngineInstance::project_name());
    }

    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives()
    {
        return {{"/", "/"}};
    }
}// namespace Engine::Platform
#endif
