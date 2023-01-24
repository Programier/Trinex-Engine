#include <Core/string_format.hpp>
#include <cstdarg>
#include <cstdio>

namespace Engine
{
    ENGINE_EXPORT std::string c_style_format(const char* text, ...)
    {
        va_list args;
        va_start(args, text);

        char buffer[1024];
        std::vsprintf(buffer, text, args);
        va_end(args);
        return buffer;
    }


    ENGINE_EXPORT std::wstring c_style_format(const wchar_t* text, ...)
    {
        va_list args;
        va_start(args, text);

        wchar_t buffer[1024];
        std::vswprintf(buffer, 1024, text, args);
        va_end(args);
        return buffer;
    }

}// namespace Engine
