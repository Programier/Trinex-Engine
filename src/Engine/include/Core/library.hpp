#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>


namespace Engine
{
    class ENGINE_EXPORT Library final
    {
    private:
        String _M_libname;
        void* _M_handle = nullptr;
        void* load_function(void* handle, const String& name);

        static void close_all();

    public:
        Library(const String& libname);

        copy_constructors_hpp(Library);
        constructor_hpp(Library);
        bool has_lib() const;


        const String& libname() const;
        void close();
        Library& load(const String& lib);

        template<typename ReturnType = void, typename... Args>
        auto get(const String& function_name)
        {
            return (ReturnType(*)(Args...))(load_function(_M_handle, function_name));
        }

        void* resolve(const String& name);
        operator bool() const;

        friend class EngineInstance;
    };
}// namespace Engine
