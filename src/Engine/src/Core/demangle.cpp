#include <Core/demangle.hpp>
#include <Core/string_functions.hpp>
#include <cxxabi.h>


namespace Engine::Demangle
{

    ENGINE_EXPORT String decode_name(const String& name)
    {
        std::size_t len;
        int_t s;
        char* ptr           = abi::__cxa_demangle(name.c_str(), 0, &len, &s);
        std::string _M_name = ptr;
        delete ptr;
        return _M_name;
    }

    ENGINE_EXPORT String decode_name(const std::type_info& info)
    {
        return decode_name(info.name());
    }
}// namespace Engine::Demangle