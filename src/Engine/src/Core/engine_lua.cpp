#include <Core/config.hpp>
#include <Core/engine_lua.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <LuaJIT/lua.hpp>


namespace Engine::Lua
{
    static sol::state* _M_lua = nullptr;

    static void make_lua_dir(String& work_dir)
    {
        work_dir = ";" + (FileManager::root_file_manager()->work_dir() / Path(engine_config.lua_scripts_dir)).string();

        if (work_dir.back() != FS::path::preferred_separator)
            work_dir.push_back(FS::path::preferred_separator);

        work_dir += "?.lua";
    }

    void Interpretter::init()
    {
        if (_M_lua)
            return;

        _M_lua = new sol::state();
        _M_lua->open_libraries();

        Interpretter::execute_string("io.stdout->setvbuf('no');");
#include "lua_code.inl"
        Interpretter::execute_string(trinex_lua_code);
    }

    void Interpretter::init_lua_dir()
    {
        String script_dir;
        make_lua_dir(script_dir);

        auto path = global_namespace()["package"]["path"];
        path      = path.get<String>() + script_dir;
    }

    void Interpretter::terminate()
    {
        if (_M_lua)
        {
            delete _M_lua;
            _M_lua = nullptr;
        }
    }

    Namespace Interpretter::namespace_of(const String& class_name, String* out_name)
    {
        size_t prev_index = 0;
        size_t index      = 0;

        sol::table parent;
        sol::table current = global_namespace().as<sol::table>();

        while ((index = class_name.find("::", prev_index)) != String::npos)
        {
            parent = std::move(current);

            String namespace_name = class_name.substr(prev_index, index - prev_index);
            prev_index            = index + 2;

            sol::object new_table = parent[namespace_name];

            if (!new_table.valid())
            {
                new_table              = _M_lua->create_table();
                parent[namespace_name] = new_table;
            }

            current = new_table.as<sol::table>();
        }

        if (out_name)
        {
            (*out_name) = class_name.substr(prev_index, class_name.length() - prev_index);
        }

        return current;
    }

    Result Interpretter::execute_string(const char* line)
    try
    {
        return _M_lua->script(line);
    }
    catch (const std::exception& e)
    {
        logger->error("LuaException: %s", e.what());
        return {};
    }

    Result Interpretter::execute_string(const String& line)
    {
        return execute_string(line.c_str());
    }

    ENGINE_EXPORT sol::global_table& Interpretter::global_namespace()
    {
        return _M_lua->globals();
    }

    ENGINE_EXPORT sol::state* Interpretter::state()
    {
        return _M_lua;
    }

}// namespace Engine::Lua
