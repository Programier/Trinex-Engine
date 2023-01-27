#pragma once

#include <Core/export.hpp>
#include <Core/engine_types.hpp>

#include <sstream>
#include <string>

namespace Engine::Strings
{

    inline std::string format(const std::string& text)
    {
        return text;
    }


    template<typename Type, typename... Args>
    std::string format(std::string text, const Type& value, const Args&... args)
    {
        auto pos = text.find("{}");
        if (pos == std::string::npos)
            return text;

        std::stringstream stream;
        stream << value;

        return format(text.replace(pos, 2, stream.str()), args...);
    }


    inline String format(const String& text)
    {
        return text;
    }


    template<typename Type, typename... Args>
    String format(String text, const Type& value, const Args&... args)
    {
        auto pos = text.find(L"{}");
        if (pos == String::npos)
            return text;

        std::wstringstream stream;
        stream << value;

        return format(text.replace(pos, 2, stream.str()), args...);
    }


    ENGINE_EXPORT std::string c_style_format(const char* text, ...);
    ENGINE_EXPORT String c_style_format(const wchar_t* text, ...);

    ENGINE_EXPORT String to_string(const std::string& str);
    ENGINE_EXPORT std::string to_std_string(const String& str);
}// namespace Engine::Strings
