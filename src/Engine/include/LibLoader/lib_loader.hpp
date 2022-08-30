#pragma once
#include <functional>
#include <string>

#define lib_function(prototype) #prototype

namespace Engine
{
    class Library final
    {
    private:
        std::string _M_libname;
        void* _M_handle = nullptr;
        static void* load_function(void* handle, const std::string& name);

        Library(void* _M_handle, const std::string& libname);

    public:
        bool has_lib() const;
        const std::string& libname() const;
        void close();

        template<typename ReturnType, typename... Args>
        std::function<ReturnType(Args...)> get(const std::string& function_name)
        {
            return std::function<ReturnType(Args...)>((ReturnType(*)(Args...))(load_function(_M_handle, function_name)));
        }

        friend Library load_library(const std::string& libname);
    };

    Library load_library(const std::string& libname);
    void close_library(const std::string& libname);
    extern std::string library_dir;
}// namespace Engine
