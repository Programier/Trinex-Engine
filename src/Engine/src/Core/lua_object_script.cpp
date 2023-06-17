#include <Core/buffer_manager.hpp>
#include <Core/lua_object_script.hpp>
#include <Core/string_functions.hpp>


namespace Engine
{
    LuaObjectScript& LuaObjectScript::load()
    {
        auto result =
                Lua::Interpretter::execute_string(Strings::format("return require('{}');", path)).get<Lua::table>();

        on_update = result.get<Lua::function>("update");
        on_ready  = result.get<Lua::function>("ready");

        return *this;
    }

    LuaObjectScript& LuaObjectScript::load(const String& script_path)
    {
        path = script_path;
        return load();
    }

    bool operator&(Archive& ar, LuaObjectScript& script)
    {
        ar& script.path;
        return static_cast<bool>(ar);
    }
}// namespace Engine
