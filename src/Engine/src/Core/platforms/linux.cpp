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

    ENGINE_EXPORT Path find_default_font_path()
    {
        FILE* pipe = popen("fc-match --format=%{file}", "r");
        Path path  = {};
        if (pipe)
        {
            char font_path[1024];
            if (fgets(font_path, sizeof(font_path), pipe) != NULL)
            {
                path = font_path;
            }
            pclose(pipe);
        }

        return path;
    }
}// namespace Engine::Platform
#endif
