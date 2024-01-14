#pragma once
#include <Core/enums.hpp>


namespace Engine::Platform
{
    ENGINE_EXPORT OperationSystemType system_type();
    ENGINE_EXPORT const char* system_name();
}// namespace Engine::Platform
