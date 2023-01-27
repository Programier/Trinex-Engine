#include <Core/decode_typeid_name.hpp>
#include <Core/string_functions.hpp>
#include <cxxabi.h>


namespace Engine
{

    static std::string decode_name_private(const std::string& name)
    {
        std::size_t len;
        int_t s;
        char* ptr = abi::__cxa_demangle(name.c_str(), 0, &len, &s);
        std::string _M_name = ptr;
        delete ptr;
        return _M_name;
    }

    ENGINE_EXPORT String decode_name(const String& name)
    {
        return Strings::to_string(decode_name_private(Strings::to_std_string(name)));
    }

    ENGINE_EXPORT String decode_name(const std::type_info& info)
    {
        return Strings::to_string(decode_name_private(info.name()));
    }
}// namespace Engine
