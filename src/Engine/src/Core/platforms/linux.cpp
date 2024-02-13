#include <Core/definitions.hpp>
#include <Core/platform.hpp>

#if PLATFORM_LINUX
namespace Engine::Platform
{
    ENGINE_EXPORT OperationSystemType system_type()
    {
        return OperationSystemType::Linux;
    }

    ENGINE_EXPORT const char* system_name()
    {
        return "Linux";
    }

    ENGINE_EXPORT Path find_root_directory(int_t argc, char** argv)
    {
        if (argc == 0)// Usually it's impossible, but just in case, let it be
            return Path("./");
        return Path(argv[0]).base_path();
    }

    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives()
    {
        return {{"/", "/"}};
    }
}// namespace Engine::Platform
#endif
