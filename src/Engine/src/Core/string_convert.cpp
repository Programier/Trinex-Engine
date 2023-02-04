#include <Core/string_functions.hpp>
#include <codecvt>
#include <locale>
#include <stdarg.h>
#include <string>


namespace Engine::Strings
{

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>& convertor()
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> _M_convertor;
        return _M_convertor;
    }

    ENGINE_EXPORT std::wstring to_wstring(const String& str)
    {
        return convertor().from_bytes(str);
    }

    ENGINE_EXPORT String c_style_format(const char* text, ...)
    {
        va_list args;
        va_start(args, text);

        char buffer[1024];
        std::vsprintf(buffer, text, args);
        va_end(args);
        return buffer;
    }
}// namespace Engine::Strings
