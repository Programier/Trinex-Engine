#include <Core/config.hpp>
#include <Core/engine_lua.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
    static lua_State* _M_lua = nullptr;

    static void make_lua_dir(String& work_dir)
    {
        work_dir = ";" + (FileManager::root_file_manager()->work_dir() / Path(engine_config.lua_scripts_dir)).string();

        if (work_dir.back() != FS::path::preferred_separator)
            work_dir.push_back(FS::path::preferred_separator);

        work_dir += "?.lua";
    }

    void LuaInterpretter::init()
    {
        _M_lua = luaL_newstate();
        luaL_openlibs(_M_lua);

        LuaInterpretter::execute_string("io.stdout->setvbuf('no');");
#include "lua_code.inl"
        LuaInterpretter::execute_string(trinex_lua_code);
    }

    void LuaInterpretter::init_lua_dir()
    {
        String script_dir;
        make_lua_dir(script_dir);

        auto path = luabridge::getGlobal(_M_lua, "package")["path"];
        path      = path.tostring() + script_dir;
    }

    void LuaInterpretter::terminate()
    {
        if (_M_lua)
        {
            lua_close(_M_lua);
            _M_lua = nullptr;
        }
    }

    luabridge::Namespace LuaInterpretter::namespace_of(const String& class_name, Vector<String>& names)
    {
        size_t prev_index = 0;
        size_t index      = 0;


        auto _namespace = luabridge::getGlobalNamespace(_M_lua);

        while ((index = class_name.find_first_of("::", prev_index)) != String::npos)
        {
            names.emplace_back(class_name.substr(prev_index, index));
            prev_index = index + 2;
            _namespace = _namespace.beginNamespace(names.back().c_str());
        }

        names.emplace_back(class_name.substr(prev_index, class_name.length() - prev_index));
        return _namespace;
    }

    LuaResult LuaInterpretter::execute_string(const char* line)
    try
    {
        int args_count = lua_gettop(_M_lua);

        if (luaL_loadstring(_M_lua, line) == LUA_OK)
        {
            lua_call(_M_lua, 0, LUA_MULTRET);
        }
        else
        {
            info_log("LuaError: %s", lua_tostring(_M_lua, -1));
            return {};
        }

        args_count = lua_gettop(_M_lua) - args_count;

        LuaResult result;
        result.reserve(args_count);

        while (args_count-- > 0)
        {
            result.insert(result.begin(), luabridge::LuaRef::fromStack(_M_lua));
        }

        return result;
    }
    catch (const std::exception& e)
    {
        logger->error("LuaException: %s", e.what());
        return {};
    }

    LuaResult LuaInterpretter::execute_string(const String& line)
    {
        return execute_string(line.c_str());
    }

    ENGINE_EXPORT luabridge::Namespace LuaInterpretter::global_namespace()
    {
        return luabridge::getGlobalNamespace(_M_lua);
    }

    ENGINE_EXPORT lua_State* LuaInterpretter::state()
    {
        return _M_lua;
    }

}// namespace Engine
