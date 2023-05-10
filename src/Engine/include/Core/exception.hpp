#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <stdexcept>

namespace Engine
{
    class ENGINE_EXPORT EngineException : public std::runtime_error
    {
    public:
        EngineException(const String& msg, int line = __builtin_LINE(), const String& file = __builtin_FILE(),
                        const String& function = __builtin_FUNCTION());
    };

    using CriticalError = std::runtime_error;
}// namespace Engine
