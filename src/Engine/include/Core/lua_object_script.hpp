#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{
    class Archive;

    class ENGINE_EXPORT LuaObjectScript final
    {
    public:
        struct ScriptFunction {
        private:
            bool _M_is_valid = false;

            ScriptFunction();

        public:
            FORCE_INLINE bool is_valid() const
            {
                return _M_is_valid;
            }

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
