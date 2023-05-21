#include <Core/config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/string_functions.hpp>
#include <algorithm>


namespace Engine
{

    EngineConfig::EngineConfig() = default;


    EngineConfig& EngineConfig::instance()
    {
        static EngineConfig config;
        return config;
    }

    EngineConfig& EngineConfig::init_callback(void (*callback)(EngineConfig*))
    {
        _M_callback = callback;
        return *this;
    }

    EngineConfig& EngineConfig::init(const String& filename)
    {
        auto loader = luabridge::getGlobal(LuaInterpretter::state(), "Engine")["load_config"];
        try
        {
            loader(filename);
        }
        catch (const std::exception& e)
        {
            error_log("Config: Failed to load config!");
        }

        if (max_gc_collected_objects < 100)
        {
            max_gc_collected_objects = 2000;
        }

        if (_M_callback)
            _M_callback(this);
        return *this;
    }

    EngineConfig& EngineConfig::save(const String& filename)
    {
        luabridge::getGlobal(LuaInterpretter::state(), "Engine")["dump_config"](filename);
        return *this;
    }

    ENGINE_EXPORT EngineConfig& engine_config = EngineConfig::instance();


#define DECLARE_CONFIG_PROP(name) config_namespace.addProperty(#name, &engine_config.name, true)

    static void on_init()
    {
        auto config_namespace = LuaInterpretter::global_namespace().beginNamespace("Engine").beginNamespace("config");

        DECLARE_CONFIG_PROP(resources_dir);
        DECLARE_CONFIG_PROP(resources_dir);
        DECLARE_CONFIG_PROP(api);
        DECLARE_CONFIG_PROP(base_commandlet);
        DECLARE_CONFIG_PROP(lua_scripts_dir);

        DECLARE_CONFIG_PROP(lz4_compression_level);
        DECLARE_CONFIG_PROP(max_gc_collected_objects);
        DECLARE_CONFIG_PROP(min_g_buffer_width);
        DECLARE_CONFIG_PROP(min_g_buffer_height);
        DECLARE_CONFIG_PROP(max_g_buffer_width);
        DECLARE_CONFIG_PROP(max_g_buffer_height);

        DECLARE_CONFIG_PROP(delete_resources_after_load);
        DECLARE_CONFIG_PROP(load_shaders_to_gpu);
        DECLARE_CONFIG_PROP(load_meshes_to_gpu);
        DECLARE_CONFIG_PROP(load_textures_to_gpu);
        DECLARE_CONFIG_PROP(enable_g_buffer);

        config_namespace.endNamespace().endNamespace();
    }

    namespace
    {
        static InitializeController a(on_init);
    }
}// namespace Engine
