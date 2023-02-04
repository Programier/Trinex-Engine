#pragma once

#include <Core/export.hpp>
#include <Core/engine_types.hpp>

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
}// namespace Engine::Strings
