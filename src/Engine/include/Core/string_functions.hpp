#pragma once

#include <Core/build.hpp>
#include <Core/engine_types.hpp>
#include <fmt/format.h>
#include <numeric>

namespace Engine::Strings
{
    using fmt::format;

    ENGINE_EXPORT String c_style_format(const char* text, ...);
    ENGINE_EXPORT StringView lstrip(const StringView& line, const StringView& chars = " \t\n\r");
    ENGINE_EXPORT StringView rstrip(const StringView& line, const StringView& chars = " \t\n\r");
    ENGINE_EXPORT StringView lstrip(const StringView& line, bool (*callback)(char ch));
    ENGINE_EXPORT StringView rstrip(const StringView& line, bool (*callback)(char ch));
    ENGINE_EXPORT String make_sentence(const StringView& line);
    ENGINE_EXPORT HashIndex hash_of(const StringView& str);

    FORCE_INLINE StringView strip(const StringView& line, const StringView& chars = " \t\n\r")
    {
        return lstrip(rstrip(line, chars), chars);
    }

    FORCE_INLINE StringView strip(const StringView& line, bool (*callback)(char ch))
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

    ENGINE_EXPORT String namespace_of(const StringView& name);
    ENGINE_EXPORT String class_name_of(const StringView& name);
    ENGINE_EXPORT StringView namespace_sv_of(const StringView& name);
    ENGINE_EXPORT StringView class_name_sv_of(const StringView& name);


    template<typename Range, typename Value = typename Range::value_type, typename FormatFunction>
    inline String join(const Range& elements, const String& delimiter, const FormatFunction& function)
    {
        if (elements.empty())
            return "";

        return std::accumulate(std::next(elements.begin()), elements.end(), format("{}", function(elements[0])),
                               [&delimiter, &function](const String& a, const Value& b) { return a + delimiter + function(b); });
    }

    template<typename Range, typename Value = typename Range::value_type>
    inline String join(const Range& elements, const String& delimiter)
    {
        static auto callback = [](const Value& value) {
            if constexpr (std::is_same_v<String, Value>)
            {
                return value;
            }
            else if constexpr (std::is_same_v<StringView, Value>)
            {
                return String(value);
            }
            else
            {
                return format("{}", value);
            }
        };

        return join(elements, delimiter, callback);
    }

    ENGINE_EXPORT bool boolean_of(const char* text, size_t len = 0);
    ENGINE_EXPORT int_t integer_of(const char* text);
    ENGINE_EXPORT float float_of(const char* text);
    ENGINE_EXPORT bool read_line(StringView& stream, StringView& out);
    ENGINE_EXPORT bool read_line(StringView& stream, StringView& out, char separator);

    FORCE_INLINE String to_code_string()
    {
        return "";
    }

    template<typename T>
    FORCE_INLINE String to_code_string(const T& value)
    {
        return format("{}", value);
    }

    template<typename First, typename Second>
    FORCE_INLINE String to_code_string(const Pair<First, Second>& value)
    {
        return format("{{{}, {}}}", to_code_string(value.first), to_code_string(value.second));
    }

    template<>
    FORCE_INLINE String to_code_string<bool>(const bool& value)
    {
        return value ? "true" : "false";
    }

    template<>
    FORCE_INLINE String to_code_string<char>(const char& value)
    {
        String result(3, '\'');
        result[1] = value;
        return result;
    }

    template<>
    FORCE_INLINE String to_code_string<const char*>(const char* const& value)
    {
        return format("\"{}\"", value);
    }

    template<>
    FORCE_INLINE String to_code_string<String>(const String& value)
    {
        return format("\"{}\"", value);
    }

    template<>
    FORCE_INLINE String to_code_string<StringView>(const StringView& value)
    {
        return format("\"{}\"", value);
    }


    template<typename T, typename... Args>
    FORCE_INLINE String to_code_string(const T& value, const Args&... args)
    {
        return format("{}, {}", value, to_code_string(args...));
    }

    template<typename T>
    FORCE_INLINE String container_to_code_string(const T& container)
    {
        auto current = container.begin();
        auto end     = container.end();

        if (current == end)
            return "{}";

        String result = format("{{{}", to_code_string(*(current++)));

        for (; current != end; ++current)
        {
            result += format(", {}", to_code_string(*current));
        }

        result += "}";
        return result;
    }

    template<template<typename...> class Container, typename... Args>
    FORCE_INLINE String to_code_string(const Container<Args...>& container)
    {
        return container_to_code_string(container);
    }

    template<template<typename, auto> class Container, typename T, auto S>
    FORCE_INLINE String to_code_string(const Container<T, S>& container)
    {
        return container_to_code_string(container);
    }

    template<typename T>
    FORCE_INLINE String to_code_string(const std::initializer_list<T>& container)
    {
        return container_to_code_string(container);
    }

}// namespace Engine::Strings
