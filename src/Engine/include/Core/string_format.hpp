#pragma once

#include <Core/export.hpp>

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


    inline std::wstring format(const std::wstring& text)
    {
        return text;
    }


    template<typename Type, typename... Args>
    std::wstring format(std::wstring text, const Type& value, const Args&... args)
    {
        auto pos = text.find(L"{}");
        if (pos == std::wstring::npos)
            return text;

        std::wstringstream stream;
        stream << value;

        return format(text.replace(pos, 2, stream.str()), args...);
    }


    ENGINE_EXPORT std::string c_style_format(const char* text, ...);
    ENGINE_EXPORT std::wstring c_style_format(const wchar_t* text, ...);

}// namespace Engine::Strings
