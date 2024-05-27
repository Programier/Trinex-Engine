#pragma once
#include <Core/enums.hpp>


namespace Engine::Platform
{
    ENGINE_EXPORT OperationSystemType system_type();
    ENGINE_EXPORT const char* system_name();
    ENGINE_EXPORT Path find_root_directory(int_t argc, const char** argv);
    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives();

    ENGINE_EXPORT void show_splash_screen(const StringView& splash_name);
    ENGINE_EXPORT void hide_splash_screen();
}// namespace Engine::Platform
