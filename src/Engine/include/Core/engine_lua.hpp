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


        static luabridge::Namespace namespace_of(const String& class_name, Vector<String>& names);

        template<typename Instance, typename BaseClass = void>
        static auto lua_class_of(const String& class_name, Vector<String>& names)
        {
            auto _n = namespace_of(class_name, names);
            if constexpr (std::is_same_v<BaseClass, void>)
            {
                return _n.beginClass<Instance>(names.back().c_str());
            }
            else
            {
                return _n.deriveClass<Instance, BaseClass>(names.back().c_str());
            }
        }

    public:
        ENGINE_EXPORT static LuaResult execute_string(const char* line);
        ENGINE_EXPORT static LuaResult execute_string(const String& line);
        ENGINE_EXPORT static luabridge::Namespace global_namespace();
        ENGINE_EXPORT static lua_State* state();

        template <typename T>
        struct EnumWrapper
        {
          static typename std::enable_if<std::is_enum<T>::value, void>::type push(lua_State* L, T value)
          {
            lua_pushnumber (L, static_cast<std::size_t> (value));
          }

          static typename std::enable_if<std::is_enum<T>::value, T>::type get(lua_State* L, int index)
          {
            return static_cast <T> (lua_tointeger (L, index));
          }
        };


        friend class EngineInstance;
        friend class Class;
    };
}// namespace Engine
