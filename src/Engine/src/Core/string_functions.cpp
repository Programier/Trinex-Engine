#include <Core/string_functions.hpp>
#include <algorithm>
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

    ENGINE_EXPORT String lstrip(String line, const String& chars)
    {
        while (line.starts_with(chars))
        {
            line = line.substr(chars.length(), line.length() - chars.length());
        }

        return line;
    }

    ENGINE_EXPORT String rstrip(String line, const String& chars)
    {
        while (line.ends_with(chars))
        {
            line = line.substr(0, line.length() - chars.length());
        }
        return line;
    }

    ENGINE_EXPORT String lstrip(String line, bool (*callback)(char ch))
    {
        String::size_type pos = 0;
        while (pos < line.size() && callback(line[pos])) pos++;
        return line.substr(pos, line.length() - pos);
    }

    ENGINE_EXPORT String rstrip(String line, bool (*callback)(char ch))
    {
        String::size_type pos = line.size() - 1;
        while (pos > 0 && callback(line[pos])) pos--;
        if (pos == 0 && callback(line[pos]))
            return "";

        return line.substr(0, pos + 1);
    }

    String replace_all(String line, const String& old, const String& new_line)
    {
        size_t pos = 0;
        while ((pos = line.find(old, pos)) != String::npos)
        {
            line.replace(pos, old.length(), new_line);
            pos += new_line.length();
        }
        return line;
    }


    ENGINE_EXPORT String& to_lower(String& line)
    {
        static auto callback = [](char value) -> char { return std::tolower(value); };
        std::transform(line.begin(), line.end(), line.begin(), callback);
        return line;
    }

    ENGINE_EXPORT String to_lower(const String& line)
    {
        String result = line;
        return to_lower(result);
    }

    ENGINE_EXPORT String& to_upper(String& line)
    {
        static auto callback = [](char value) -> char { return std::toupper(value); };
        std::transform(line.begin(), line.end(), line.begin(), callback);
        return line;
    }

    ENGINE_EXPORT String to_upper(const String& line)
    {
        String result = line;
        return to_upper(result);
    }
}// namespace Engine::Strings
