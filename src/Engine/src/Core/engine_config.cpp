#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/string_functions.hpp>
#include <algorithm>


namespace Engine
{

    EngineConfig::EngineConfig()
    {
        auto callback = std::bind(&EngineConfig::on_config_load, this);
        _M_callback   = [callback]() { callback(); };
    }

    void EngineConfig::write_lua_object(void* object)
    {
        Lua::object* lua_object = reinterpret_cast<Lua::object*>(object);
        (*lua_object)           = Lua::make_object(Lua::Interpretter::state()->lua_state(), this);
    }

    void EngineConfig::on_config_load()
    {
        if (max_gc_collected_objects < 100)
        {
            max_gc_collected_objects = 2000;
        }
    }


    EngineConfig& EngineConfig::instance()
    {
        static EngineConfig config;
        return config;
    }

    ENGINE_EXPORT EngineConfig& engine_config = EngineConfig::instance();


#define DECLARE_CONFIG_PROP(name) config_namespace.set(#name, &EngineConfig::name)

    static void on_init()
    {
        auto config_namespace = Lua::Interpretter::lua_class_of<EngineConfig>("Engine::EngineConfig");

        DECLARE_CONFIG_PROP(resources_dir);
        DECLARE_CONFIG_PROP(api);
        DECLARE_CONFIG_PROP(base_commandlet);
        DECLARE_CONFIG_PROP(lua_scripts_dir);
        DECLARE_CONFIG_PROP(libraries_dir);
        DECLARE_CONFIG_PROP(shader_compilers_lib);
        DECLARE_CONFIG_PROP(shader_compiler);

        DECLARE_CONFIG_PROP(lz4_compression_level);
        DECLARE_CONFIG_PROP(max_gc_collected_objects);
        DECLARE_CONFIG_PROP(min_g_buffer_width);
        DECLARE_CONFIG_PROP(min_g_buffer_height);
        DECLARE_CONFIG_PROP(max_g_buffer_width);
        DECLARE_CONFIG_PROP(max_g_buffer_height);
        DECLARE_CONFIG_PROP(back_buffer_count);

        DECLARE_CONFIG_PROP(delete_resources_after_load);
        DECLARE_CONFIG_PROP(load_shaders_to_gpu);
        DECLARE_CONFIG_PROP(load_meshes_to_gpu);
        DECLARE_CONFIG_PROP(load_textures_to_gpu);
        DECLARE_CONFIG_PROP(enable_g_buffer);

        Lua::Interpretter::namespace_of("Engine::").set("config", &engine_config);
    }

    namespace
    {
        static InitializeController a(on_init);
    }
}// namespace Engine
