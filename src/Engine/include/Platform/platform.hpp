#pragma once
#include <Core/enums.hpp>


namespace Engine::Platform
{
    ENGINE_EXPORT OperationSystemType system_type();
    ENGINE_EXPORT const char* system_name();
    ENGINE_EXPORT Path find_root_directory(int_t argc, char** argv);
    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives();
}// namespace Engine::Platform
