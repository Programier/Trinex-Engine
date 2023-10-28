#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>


namespace Engine
{
    ENGINE_EXPORT EngineConfig engine_config;

    EngineConfig& EngineConfig::update()
    {
        const auto& engine_json = global_config.checked_get("Engine").checked_get<JSON::JsonObject>();

        resources_dir        = engine_json.checked_get_value<JSON::JsonString>("resources_dir");
        api                  = engine_json.checked_get_value<JSON::JsonString>("api");
        lua_scripts_dir      = engine_json.checked_get_value<JSON::JsonString>("lua_scripts_dir");
        libraries_dir        = engine_json.checked_get_value<JSON::JsonString>("libraries_dir");
        shader_compilers_lib = engine_json.checked_get_value<JSON::JsonString>("shader_compilers_lib");
        shader_compiler      = engine_json.checked_get_value<JSON::JsonString>("shader_compiler");
        window_system        = engine_json.checked_get_value<JSON::JsonString>("window_system");

        lz4_compression_level    = engine_json.checked_get_value<JSON::JsonInt>("lz4_compression_level");
        max_gc_collected_objects = engine_json.checked_get_value<JSON::JsonInt>("max_gc_collected_objects");
        min_g_buffer_width       = engine_json.checked_get_value<JSON::JsonInt>("min_g_buffer_width");
        min_g_buffer_height      = engine_json.checked_get_value<JSON::JsonInt>("min_g_buffer_height");
        max_g_buffer_width       = engine_json.checked_get_value<JSON::JsonInt>("max_g_buffer_width");
        max_g_buffer_height      = engine_json.checked_get_value<JSON::JsonInt>("max_g_buffer_height");
        back_buffer_count        = engine_json.checked_get_value<JSON::JsonInt>("back_buffer_count");

        delete_resources_after_load = engine_json.checked_get_value<JSON::JsonBool>("delete_resources_after_load");
        load_shaders_to_gpu         = engine_json.checked_get_value<JSON::JsonBool>("load_shaders_to_gpu");
        load_meshes_to_gpu          = engine_json.checked_get_value<JSON::JsonBool>("load_meshes_to_gpu");
        load_textures_to_gpu        = engine_json.checked_get_value<JSON::JsonBool>("load_textures_to_gpu");
        use_deffered_rendering      = engine_json.checked_get_value<JSON::JsonBool>("use_deffered_rendering");
        return *this;
    }

}// namespace Engine
