#pragma once
#include <Core/engine_types.hpp>
#include <Core/string_functions.hpp>
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

#define trinex_check(expression, msg)                                                                                  \
    if (!(expression))                                                                                                 \
    throw Engine::EngineException(Strings::format("Assertion failed: {}") + #expression)
}// namespace Engine
