#pragma once
#include <Core/engine_lua.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>


namespace Engine
{
    class Archive;

    class ENGINE_EXPORT LuaObjectScript final
    {
    public:
        Lua::function on_ready;
        Lua::function on_update;
        String path;

        LuaObjectScript& load();
        LuaObjectScript& load(const String& script_path);

        friend bool operator&(Archive& ar, LuaObjectScript& script);
    };

    bool operator&(Archive& ar, LuaObjectScript& script);
}// namespace Engine
