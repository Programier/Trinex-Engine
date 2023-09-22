#include <Core/definitions.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Core/structures.hpp>

#if PLATFORM_WINDOWS
#include <windows.h>
#elif PLATFORM_LINUX
#include <cstdlib>
#elif PLATFORM_ANDROID
#endif


namespace Engine
{
#if PLATFORM_WINDOWS
    void create_system_notify(const NotifyCreateInfo& info)
    {
        MessageBox(NULL, info.message.c_str(), info.title.c_str(), MB_ICONINFORMATION);
    }
#elif PLATFORM_LINUX
    void create_system_notify(const NotifyCreateInfo& info)
    {
        String command = Strings::format("notify-send '{}' '{}'", info.title, info.message);

        if (!info.icon_path.empty())
        {
            command += Strings::format(" --icon='{}'", info.icon_path.c_str());
        }

        if (!info.app_name.empty())
        {
            command += Strings::format(" --app-name='{}'", info.app_name);
        }

        info_log("Notify", "Command: %s", command.c_str());
        std::system(command.c_str());
    }
#elif PLATFORM_ANDROID
    void create_system_notify(const NotifyCreateInfo& info)
    {}
#else
    void create_system_notify(const NotifyCreateInfo& info)
    {}
#endif
}// namespace Engine
