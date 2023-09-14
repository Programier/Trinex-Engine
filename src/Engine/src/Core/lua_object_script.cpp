#include <Core/buffer_manager.hpp>
#include <Core/lua_object_script.hpp>
#include <Core/string_functions.hpp>


namespace Engine
{

    LuaObjectScript::ScriptFunction::ScriptFunction()
    {}

    LuaObjectScript& LuaObjectScript::load()
    {
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
