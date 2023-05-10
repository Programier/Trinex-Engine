#include <Core/exception.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
    static inline String get_message(const String& msg, int line, const String& file, const String& function)
    {
        String format = "Critical error at:\n\tFile: {}\n\tFunction: {}\n\tLine: {}\n\tMessage: {}\n";
        return Strings::format(format, file, function, line, msg);
    }

    EngineException::EngineException(const String& msg, int line, const String& file, const String& function)
        : std::runtime_error(get_message(msg, line, file, function))
    {}
}// namespace Engine
