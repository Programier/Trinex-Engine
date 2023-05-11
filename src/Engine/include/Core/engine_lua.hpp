#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <LuaJIT/lua.hpp>

#include <LuaBridge/LuaBridge.h>

namespace Engine
{

    using LuaResult = Vector<luabridge::LuaRef>;
    struct ENGINE_EXPORT LuaInterpretter {
    private:
        ENGINE_EXPORT static void init();
        ENGINE_EXPORT static void init_lua_dir();
        ENGINE_EXPORT static void terminate();


        static luabridge::Namespace namespace_of(const String& class_name, size_t& namespaces, String& pure_class_name);

        template<typename Instance>
        static auto lua_class_of(const String& class_name, size_t& namespaces)
        {
            String pure_class_name;
            namespaces = 0;
            auto _n = namespace_of(class_name, namespaces, pure_class_name);
            return _n.beginClass<Instance>(pure_class_name.c_str());
        }
    public:

        ENGINE_EXPORT static LuaResult execute_string(const char* line);
        ENGINE_EXPORT static LuaResult execute_string(const String& line);
        ENGINE_EXPORT static luabridge::Namespace global_namespace();
        ENGINE_EXPORT static lua_State* state();

        friend class EngineInstance;
        friend class Class;
    };
}// namespace Engine
