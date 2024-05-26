#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>

namespace Engine
{
    ENGINE_EXPORT EngineConfig engine_config;

    EngineConfig& EngineConfig::init()
    {
        global_config.load("resources/configs/engine.json");
        update();
        return *this;
    }

    EngineConfig& EngineConfig::update()
    {
        const auto& engine_json = global_config.checked_get("Engine").checked_get<JSON::JsonObject>();

        external_system_libraries.clear();
        systems.clear();
        languages.clear();

        engine_json.checked_get("external_system_libraries").copy_to_array(external_system_libraries, JSON::ValueType::String);
        engine_json.checked_get("systems").copy_to_array(systems, JSON::ValueType::String);
        engine_json.checked_get("languages").copy_to_array(languages, JSON::ValueType::String);


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

        // lz4_compression_level  = engine_json.checked_get_value<JSON::JsonInt>("lz4_compression_level", 1);
        // gc_max_object_per_tick = engine_json.checked_get_value<JSON::JsonInt>("gc_max_object_per_tick", 10);
        // fps_limit              = engine_json.checked_get_value<JSON::JsonInt>("fps_limit", 60);

        return *this;
    }


    template<typename Type>
    static void load_argument(const char* arg_name, const char* value_name, Arguments::Type type, const Type& default_value)
    {
        Arguments::Argument* arg = Arguments::find(arg_name);
        if (arg && arg->type == type)
        {
            if constexpr (std::is_same_v<Type, String>)
            {
                ConfigManager::set(value_name, arg->get<Type>());
            }
            else if constexpr (std::is_same_v<Type, int_t>)
            {
                ConfigManager::set(value_name, std::stoi(arg->get<String>()));
            }
        }
        else
        {
            ConfigManager::set(value_name, default_value);
        }
    }


    static void on_init()
    {
        load_argument<String>("e_project_name", "Engine::project_name", Arguments::Type::String, "Trinex Engine");
        load_argument<String>("engine", "Engine::engine", Arguments::Type::String, "Engine::EngineInstance");
        load_argument<String>("e_api", "Engine::api", Arguments::Type::String, "OpenGLES");
        load_argument<String>("e_shader_cache_dir", "Engine::shader_cache_dir", Arguments::Type::String, "ShaderCache");
        load_argument<String>("e_config_dir", "Engine::config_dir", Arguments::Type::String, "resources/configs");
        load_argument<String>("e_assets_dir", "Engine::assets_dir", Arguments::Type::String, "resources/assets");
        load_argument<String>("e_scripts_dir", "Engine::scripts_dir", Arguments::Type::String, "resources/scripts");
        load_argument<String>("e_libraries_dir", "Engine::libraries_dir", Arguments::Type::String, "libs");
        load_argument<String>("e_localization_dir", "Engine::localization_dir", Arguments::Type::String,
                              "resources/localization");
        load_argument<String>("e_shaders_dir", "Engine::shaders_dir", Arguments::Type::String, "resources/shaders");
        load_argument<String>("e_default_language", "Engine::default_language", Arguments::Type::String, "eng");
        load_argument<String>("e_current_language", "Engine::current_language", Arguments::Type::String, "eng");
        load_argument<String>("e_window_system", "Engine::window_system", Arguments::Type::String, "SDL2");
        load_argument<int_t>("e_lz4_compression_level", "Engine::lz4_compression_level", Arguments::Type::String, 1);
        load_argument<int_t>("e_gc_max_object_per_tick", "Engine::gc_max_object_per_tick", Arguments::Type::String, 10);
        load_argument<int_t>("e_fps_limit", "Engine::fps_limit", Arguments::Type::String, 60);
    }

    // Initialize config
    static ScriptEngineInitializeController initialize_configs(on_init, "EngineConfig");

}// namespace Engine
