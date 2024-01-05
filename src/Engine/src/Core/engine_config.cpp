#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>

namespace Engine
{
    ENGINE_EXPORT EngineConfig engine_config;


    EngineConfig& EngineConfig::update()
    {
        const auto& engine_json = global_config.checked_get("Engine").checked_get<JSON::JsonObject>();

        engine_json.checked_get("external_system_libraries").copy_to_array(external_system_libraries, JSON::ValueType::String);
        engine_json.checked_get("systems").copy_to_array(systems, JSON::ValueType::String);

        resources_dir        = engine_json.checked_get_value<JSON::JsonString>("resources_dir", "resources");
        api                  = engine_json.checked_get_value<JSON::JsonString>("api", "OpenGLES");
        scripts_dir          = engine_json.checked_get_value<JSON::JsonString>("scripts_dir", "scripts");
        libraries_dir        = engine_json.checked_get_value<JSON::JsonString>("libraries_dir", "libs");
        shader_compilers_lib = engine_json.checked_get_value<JSON::JsonString>("shader_compilers_lib");
        shader_compiler      = engine_json.checked_get_value<JSON::JsonString>("shader_compiler");
        window_system        = engine_json.checked_get_value<JSON::JsonString>("window_system", "SDL2");

#if TRINEX_WITH_SKIP_JIT_INSTRUCTIONS
        {
            auto object = engine_json.checked_get_value<JSON::JsonObject>("jit_skip_instructions");
            for (auto& [name, array] : object)
            {
                Vector<int> indices;
                array.copy_to_array(indices, JSON::ValueType::Integer);
                jit_skip_instructions[name] = std::move(indices);
            }
        }
#endif

        lz4_compression_level    = engine_json.checked_get_value<JSON::JsonInt>("lz4_compression_level", 9);
        max_gc_collected_objects = engine_json.checked_get_value<JSON::JsonInt>("max_gc_collected_objects", 2000);
        fps_limit                = engine_json.checked_get_value<JSON::JsonInt>("fps_limit", 60);
        enable_jit               = engine_json.checked_get_value<JSON::JsonBool>("enable_jit", true);

        {
            Arguments::Argument* arg = engine_instance->args().find("config_dir");
            if (arg && arg->type == Arguments::Type::String)
            {
                config_dir = arg->get<String>();
            }
            else
            {
                config_dir = "configs";
            }
        }
        return *this;
    }


}// namespace Engine
