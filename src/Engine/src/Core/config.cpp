#include <Core/config.hpp>
#include <Core/engine_lua.hpp>


namespace Engine
{
    Config& Config::load_config(const String& filename)
    {
        sol::object object;
        write_lua_object(&object);

        auto loader = Lua::Interpretter::global_namespace()["Engine"]["load_config"];
        try
        {
            if (!loader.is<sol::function>())
            {
                logger->error("Config", "Cannot find loader in Lua!");
                return *this;
            }

            loader(filename, object);
        }
        catch (const std::exception& e)
        {
            error_log("Config", "Failed to load config!");
        }

        if (_M_callback)
            _M_callback();
        return *this;
    }

    Config& Config::save_config(const String& filename)
    {
        sol::object object;
        write_lua_object(&object);
        Lua::Interpretter::global_namespace()["Engine"]["dump_config"](filename, object);
        return *this;
    }

    Config::~Config()
    {}
}// namespace Engine
