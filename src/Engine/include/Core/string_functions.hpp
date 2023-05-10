#pragma once

#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/predef.hpp>

#include <sstream>
#include <string>

namespace Engine::Strings
{
    inline String format(const String& text)
    {
        return text;
    }


    template<typename Type, typename... Args>
    String format(String text, const Type& value, const Args&... args)
    {
        auto pos = text.find("{}");
        if (pos == String::npos)
            return text;

        std::stringstream stream;
        stream << value;

        return format(text.replace(pos, 2, stream.str()), args...);
    }


    ENGINE_EXPORT String c_style_format(const char* text, ...);
    ENGINE_EXPORT std::wstring to_wstring(const String& str);
    ENGINE_EXPORT String lstrip(String line, const String& chars = " ");
    ENGINE_EXPORT String rstrip(String line, const String& chars = " ");
    ENGINE_EXPORT String lstrip(String line, bool(*callback)(char ch));
    ENGINE_EXPORT String rstrip(String line, bool(*callback)(char ch));

    FORCE_INLINE String strip(String line, const String& chars = " ")
    {
        return lstrip(rstrip(line, chars), chars);
    }

    FORCE_INLINE String strip(String line, bool(*callback)(char ch))
    {
        return lstrip(rstrip(line, callback), callback);
    }

    ENGINE_EXPORT String replace_all(String line, const String& old, const String& new_line);
    ENGINE_EXPORT String& to_lower(String& line);
    ENGINE_EXPORT String to_lower(const String& line);
    ENGINE_EXPORT String& to_upper(String& line);
    ENGINE_EXPORT String to_upper(const String& line);
}// namespace Engine::Strings
