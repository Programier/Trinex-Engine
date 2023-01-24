#include <Core/string_convert.hpp>
#include <codecvt>
#include <locale>
#include <string>


namespace Engine::Strings
{

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>& convertor()
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> _M_convertor;
        return _M_convertor;
    }

    ENGINE_EXPORT std::wstring to_wstring(const std::string& str)
    {
        return convertor().from_bytes(str);
    }

    ENGINE_EXPORT std::string to_string(const std::wstring& str)
    {
        return convertor().to_bytes(str);
    }
}// namespace Engine::Strings
