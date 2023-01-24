#include <Core/decode_typeid_name.hpp>
#include <Core/string_convert.hpp>
#include <cxxabi.h>


namespace Engine
{
    ENGINE_EXPORT String decode_name(const std::type_info& info)
    {
        std::size_t len;
        int s;
        char* ptr = abi::__cxa_demangle(info.name(), 0, &len, &s);
        std::string _M_name = ptr;
        delete ptr;
        return Strings::to_wstring(_M_name);
    }
}// namespace Engine
