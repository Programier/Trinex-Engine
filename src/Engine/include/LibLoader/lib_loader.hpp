#pragma once
#include <functional>
#include <string>
#include <Core/export.hpp>
#include <Core/implement.hpp>

#define lib_function(prototype) #prototype

namespace Engine
{
    CLASS Library final
    {
    private:
        std::string _M_libname;
        void* _M_handle = nullptr;
        void* load_function(void* handle, const std::string& name);

        Library(void* _M_handle, const std::string& libname);

    public:
        copy_constructors_hpp(Library);
        constructor_hpp(Library);
        bool has_lib() const;
        const std::string& libname() const;
        void close();

        template<typename ReturnType, typename... Args>
        auto get(const std::string& function_name)
        {
            return (ReturnType(*)(Args...))(load_function(_M_handle, function_name));
        }

        void* resolve(const std::string& name);

        friend ENGINE_EXPORT Library load_library(const std::string& libname);
    };

    ENGINE_EXPORT Library load_library(const std::string& libname);
    ENGINE_EXPORT void close_library(const std::string& libname);
    extern ENGINE_EXPORT std::string library_dir;
}// namespace Engine
