#pragma once
#include <Core/engine_lua.hpp>


namespace Engine
{
    class Archive;

    class ENGINE_EXPORT LuaObjectScript final
    {
    public:
        struct ScriptFunction {
        private:
            Lua::function _M_function;
            Lua::function_result _M_result;
            bool _M_is_valid = false;


            void operator = (Lua::function&& function);

            ScriptFunction();

        public:
            FORCE_INLINE bool is_valid() const
            {
                return _M_is_valid;
            }

            FORCE_INLINE const Lua::function_result& last_result() const
            {
                return _M_result;
            }

            void operator()(const Lua::object& object);
            void operator()(Lua::object&& object);

            friend class LuaObjectScript;
        };

        ScriptFunction on_ready;
        ScriptFunction on_update;

        String path;

        LuaObjectScript& load();
        LuaObjectScript& load(const String& script_path);

        friend bool operator&(Archive& ar, LuaObjectScript& script);
    };

    bool operator&(Archive& ar, LuaObjectScript& script);
}// namespace Engine
