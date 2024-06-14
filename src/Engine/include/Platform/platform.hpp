#pragma once
#include <Core/enums.hpp>


namespace Engine
{
    class Window;
    struct WindowConfig;
    struct MonitorInfo;

    namespace Platform
    {
        ENGINE_EXPORT OperationSystemType system_type();
        ENGINE_EXPORT const char* system_name();
        ENGINE_EXPORT Path find_root_directory();
        ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives();

        namespace WindowManager
        {
            ENGINE_EXPORT void initialize();
            ENGINE_EXPORT void terminate();

            ENGINE_EXPORT Window* create_window(const WindowConfig* config);
            ENGINE_EXPORT void destroy_window(Window* interface);
            ENGINE_EXPORT bool mouse_relative_mode();
            ENGINE_EXPORT void mouse_relative_mode(bool flag);
            ENGINE_EXPORT void update_monitor_info(MonitorInfo& info);
            ENGINE_EXPORT void pool_events();
            ENGINE_EXPORT void wait_for_events();
        }// namespace WindowManager

        namespace LibraryLoader
        {
            ENGINE_EXPORT void* load_library(const String& name);
            ENGINE_EXPORT void close_library(void* handle);
            ENGINE_EXPORT void* find_function(void* handle, const String& name);
        }// namespace LibraryLoader

    }// namespace Platform
}// namespace Engine
