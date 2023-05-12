#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <LuaJIT/lua.hpp>
#include <Core/logger.hpp>
#include <LuaBridge/LuaBridge.h>

namespace Engine
{

    using LuaResult = Vector<luabridge::LuaRef>;
    struct ENGINE_EXPORT LuaInterpretter {
    private:
        ENGINE_EXPORT static void init();
        ENGINE_EXPORT static void init_lua_dir();
        ENGINE_EXPORT static void terminate();


        static luabridge::Namespace namespace_of(const String& class_name, Vector<String>& names);

        template<typename Instance>
        static auto lua_class_of(const String& class_name, Vector<String>& names, void const* const key = nullptr)
        {
            auto _n = namespace_of(class_name, names);
            if (key == nullptr)
            {
                return _n.beginClass<Instance>(names.back().c_str());
            }
            else
            {
                try
                {
                    return luabridge::Namespace::Class<Instance>(names.back().c_str(), _n, key);
                }
                catch (std::exception& e)
                {
                    error_log("LuaClassRegister: Failed to register class '%s': %s", names.back().c_str(), e.what());
                    throw e;
                }
            }
        }

    public:
        ENGINE_EXPORT static LuaResult execute_string(const char* line);
        ENGINE_EXPORT static LuaResult execute_string(const String& line);
        ENGINE_EXPORT static luabridge::Namespace global_namespace();
        ENGINE_EXPORT static lua_State* state();

        template<typename T>
        struct EnumWrapper {
            static typename std::enable_if<std::is_enum<T>::value, void>::type push(lua_State* L, T value)
            {
                lua_pushnumber(L, static_cast<std::size_t>(value));
            }

            static typename std::enable_if<std::is_enum<T>::value, T>::type get(lua_State* L, int index)
            {
                return static_cast<T>(lua_tointeger(L, index));
            }
        };


        friend class EngineInstance;
        friend class Class;
    };
}// namespace Engine
