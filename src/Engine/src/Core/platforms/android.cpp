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

    ENGINE_EXPORT Path find_default_font_path()
    {
        return {};
    }
}// namespace Engine::Platform
#endif
