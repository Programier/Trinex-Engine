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
}// namespace Engine::Platform
#endif
