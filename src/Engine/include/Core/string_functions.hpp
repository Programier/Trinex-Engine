#pragma once

#include <Core/build.hpp>
#include <Core/engine_types.hpp>
#include <fmt/format.h>
#include <numeric>
#include <sstream>

namespace Engine::Strings
{
    using fmt::format;

    ENGINE_EXPORT String c_style_format(const char* text, ...);
    ENGINE_EXPORT std::wstring to_wstring(const String& str);
    ENGINE_EXPORT std::wstring to_wstring(const char* str);
    ENGINE_EXPORT String lstrip(String line, const String& chars = " ");
    ENGINE_EXPORT String rstrip(String line, const String& chars = " ");
    ENGINE_EXPORT String lstrip(String line, bool (*callback)(char ch));
    ENGINE_EXPORT String rstrip(String line, bool (*callback)(char ch));
    ENGINE_EXPORT String make_sentence(const StringView& line);
    ENGINE_EXPORT HashIndex hash_of(const StringView& str);


    FORCE_INLINE String strip(String line, const String& chars = " ")
    {
        return lstrip(rstrip(line, chars), chars);
    }

    FORCE_INLINE String strip(String line, bool (*callback)(char ch))
    {
        return lstrip(rstrip(line, callback), callback);
    }

    ENGINE_EXPORT String replace_all(StringView line, StringView old, StringView new_line);
    ENGINE_EXPORT String& to_lower(String& line);
    ENGINE_EXPORT String to_lower(const String& line);
    ENGINE_EXPORT String& to_upper(String& line);
    ENGINE_EXPORT String to_upper(const String& line);

    ENGINE_EXPORT const char* strnstr(const char* haystack, size_t haystack_len, const char* needle, size_t needle_len);
    ENGINE_EXPORT Vector<String> split(const StringView& line, char delimiter = ' ');
    ENGINE_EXPORT Vector<String> split(const StringView& line, const StringView& delimiter);

    template<typename T>
    FORCE_INLINE T convert(const char* line)
    {
        T value;
        std::stringstream(line) >> value;
        return value;
    }

    template<typename T>
    FORCE_INLINE T convert(const String& line)
    {
        T value;
        std::stringstream(line) >> value;
        return value;
    }

    ENGINE_EXPORT String namespace_of(const StringView& name);
    ENGINE_EXPORT String class_name_of(const StringView& name);
    ENGINE_EXPORT StringView namespace_sv_of(const StringView& name);
    ENGINE_EXPORT StringView class_name_sv_of(const StringView& name);

    template<typename Range, typename Value = typename Range::value_type>
    inline String join(const Range& elements, const String& delimiter)
    {
        if (elements.empty())
            return "";

        return std::accumulate(std::next(elements.begin()), elements.end(), elements[0],
                               [&delimiter](const std::string& a, const Value& b) { return a + delimiter + b; });
    }
}// namespace Engine::Strings
