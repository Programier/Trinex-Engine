#include <Core/memory.hpp>
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

    ENGINE_EXPORT std::wstring to_wstring(const char* str)
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

    static bool (*insert_space[])(char, char, String& to) = {
            [](char, char ch, String& to) {
                if (ch == '_')
                {
                    to.push_back(' ');
                    return true;
                }
                return false;
            },
            [](char prev, char ch, String& to) -> bool {
                if (std::isdigit(ch) && !isdigit(prev))
                {
                    to.push_back(' ');
                    to.push_back(ch);
                    return true;
                }
                else if (std::isupper(ch) && std::islower(prev) && !std::isdigit(prev))
                {
                    to.push_back(' ');
                    to.push_back(ch);
                    return true;
                }

                return false;
            },
    };


    ENGINE_EXPORT String make_sentence(const String& line)
    {
        std::string result;
        result.reserve(line.size());
        char prev_char = '\0';

        for (char ch : line)
        {
            bool inserted = false;
            if (ch != ' ')
            {
                for (size_t i = 0; i < ARRAY_SIZE(insert_space) && !inserted; ++i)
                {
                    inserted = insert_space[i](prev_char, ch, result);
                }
            }

            if (!inserted)
            {
                result.push_back(ch);
            }

            prev_char = ch;
        }

        return result;
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

    ENGINE_EXPORT const char* strnstr(const char* haystack, size_t haystack_len, const char* needle, size_t needle_len)
    {
        return reinterpret_cast<const char*>(memory_search(reinterpret_cast<const byte*>(haystack), haystack_len,
                                                           reinterpret_cast<const byte*>(needle), needle_len));
    }
}// namespace Engine::Strings
