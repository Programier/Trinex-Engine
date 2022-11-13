#pragma once

#include <string>
#include <sstream>

namespace Engine::Strings
{
    std::string format(const std::string text)
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
}
