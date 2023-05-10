#include <Core/config.hpp>
#include <Core/engine_lua.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    //    struct ENGINE_EXPORT LuaInterpretter
    //    {
    //    private:
    //        ENGINE_EXPORT static void init();
    //        ENGINE_EXPORT static void terminate();
    //    public:
    //    };

    static lua_State* _M_lua = nullptr;

    static void make_lua_dir(String& work_dir)
    {
        work_dir = ";" + FileManager::root_file_manager()->work_dir();
        if (work_dir.back() != '/')
            work_dir.push_back('/');
        work_dir += engine_config.lua_scripts_dir;

        if (work_dir.back() != '/')
            work_dir.push_back('/');

        work_dir += "?.lua";
    }

    void LuaInterpretter::init()
    {
        _M_lua = luaL_newstate();
        luaL_openlibs(_M_lua);

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

    luabridge::Namespace LuaInterpretter::namespace_of(const String& class_name, size_t& namespaces,
                                                       String& pure_class_name)
    {
        size_t prev_index = 0;
        size_t index      = 0;


        auto _namespace = luabridge::getGlobalNamespace(_M_lua);

        while ((index = class_name.find_first_of("::", prev_index)) != String::npos)
        {
            String namespace_name = class_name.substr(prev_index, index);
            prev_index            = index + 2;

            _namespace = _namespace.beginNamespace(namespace_name.c_str());
            ++namespaces;
        }

        pure_class_name = class_name.substr(prev_index, class_name.length() - prev_index);
        return _namespace;
    }

    LuaResult LuaInterpretter::execute_string(const char* line)
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

        while (args_count > 0)
        {
            result.insert(result.begin(), luabridge::LuaRef::fromStack(_M_lua));
        }

        return result;
    }

    LuaResult LuaInterpretter::execute_string(const String& line)
    {
        return execute_string(line.c_str());
    }
}// namespace Engine
