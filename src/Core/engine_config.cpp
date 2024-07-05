#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>

namespace Engine
{
    // static void on_pre_init()
    // {
    //     auto update = [](const char* arg_name, const char* value_name, const char* value) {
    //         if (!ConfigManager::is_exist(value_name))
    //         {
    //             Arguments::Argument* arg = Arguments::find(arg_name);

    //             if (arg && arg->type == Arguments::Type::String)
    //             {
    //                 ConfigManager::set(value_name, arg->get<Arguments::StringType>());
    //             }
    //             else
    //             {
    //                 ConfigManager::set(value_name, StringView(value));
    //             }
    //         }
    //     };

    //     update("e_configs_dir", "Engine::configs_dir", "resources/configs");
    //     update("e_project_name", "Engine::project_name", "TrinexEngine");
    // }

    // static Vector<String> get_libs_from_args()
    // {
    //     Arguments::Argument* arg = Arguments::find("e_libs");
    //     if (arg && arg->type == Arguments::Type::Array)
    //     {
    //         return arg->get<Arguments::ArrayType>();
    //     }

    //     return {};
    // }

    // static Vector<String> get_libs_from_config()
    // {
    //     return ConfigManager::get_string_array("Engine::libs");
    // }

    // static Vector<String> merge_array(Vector<String> a, const Vector<String>& b)
    // {
    //     a.insert(a.end(), b.begin(), b.end());
    //     return a;
    // }

    // static void on_init()
    // {
    //     // After loading configs we can override values by command line or set default value

    //     //////////////////////////////// ENGINE PART ////////////////////////////////
    //     ConfigManager::load_string_argument<String>("e_project_name", "Engine::project_name", "TrinexEngine");
    //     ConfigManager::load_string_argument<String>("e_configs_dir", "Engine::configs_dir", "resources/configs");
    //     ConfigManager::load_string_argument<String>("engine", "Engine::engine", "Engine::EngineInstance");
    //     ConfigManager::load_string_argument<String>("e_api", "Engine::api", "OpenGLES");
    //     ConfigManager::load_string_argument<String>("e_shader_cache_dir", "Engine::shader_cache_dir", "ShaderCache");
    //     ConfigManager::load_string_argument<String>("e_assets_dir", "Engine::assets_dir", "resources/assets");
    //     ConfigManager::load_string_argument<String>("e_scripts_dir", "Engine::scripts_dir", "resources/scripts");
    //     ConfigManager::load_string_argument<String>("e_libraries_dir", "Engine::libraries_dir", "libs");
    //     ConfigManager::load_string_argument<String>("e_localization_dir", "Engine::localization_dir", "resources/localization");
    //     ConfigManager::load_string_argument<String>("e_shaders_dir", "Engine::shaders_dir", "resources/shaders");
    //     ConfigManager::load_string_argument<String>("e_default_language", "Engine::default_language", "eng");
    //     ConfigManager::load_string_argument<String>("e_current_language", "Engine::current_language", "eng");
    //     ConfigManager::load_string_argument<String>("e_version", "Engine::version", "Trinex Engine 1.0");
    //     ConfigManager::load_string_argument<int_t>("e_lz4_compression_level", "Engine::lz4_compression_level", 1);
    //     ConfigManager::load_string_argument<int_t>("e_gc_max_object_per_tick", "Engine::gc_max_object_per_tick", 10);
    //     ConfigManager::load_string_argument<int_t>("e_fps_limit", "Engine::fps_limit", 60);

    //     ConfigManager::load_array_argument<String>("e_languages", "Engine::languages", {"eng"});
    //     ConfigManager::load_array_argument<String>("e_systems", "Engine::systems", {});
    //     ConfigManager::set("Engine::libs", merge_array(get_libs_from_config(), get_libs_from_args()));

    //     //////////////////////////////// WINDOWS PART ////////////////////////////////
    //     ConfigManager::load_string_argument<String>("w_title", "Window::title", "Trinex Engine");
    //     ConfigManager::load_string_argument<String>("w_client", "Window::client", "");
    //     ConfigManager::load_string_argument<int_t>("w_size_x", "Window::size_x", 1280);
    //     ConfigManager::load_string_argument<int_t>("w_size_y", "Window::size_y", 720);
    //     ConfigManager::load_string_argument<int_t>("w_pos_x", "Window::pos_x", -1);
    //     ConfigManager::load_string_argument<int_t>("w_pos_y", "Window::pos_y", -1);
    //     ConfigManager::load_string_argument<bool>("w_vsync", "Window::vsync", true);
    //     ConfigManager::load_string_argument<int_t>("w_attributes", "Window::attributes", {});
    //     ConfigManager::load_array_argument<int_t>("w_orientations", "Window::orientations", {});

    //     //////////////////////////////// SPLASH PART ////////////////////////////////
    //     ConfigManager::load_string_argument<bool>("e_show_splash", "Engine::Splash::show", static_cast<bool>(!PLATFORM_ANDROID));
    //     ConfigManager::load_string_argument<String>("e_splash_image", "Engine::Splash::image", "resources/splash/splash.png");
    //     ConfigManager::load_string_argument<String>("e_splash_font", "Engine::Splash::font_file", "");
    //     ConfigManager::load_string_argument<int_t>("e_splash_startup_text_size", "Engine::Splash::startup_text_size", 14);
    //     ConfigManager::load_string_argument<int_t>("e_splash_version_text_size", "Engine::Splash::version_text_size", 14);
    //     ConfigManager::load_string_argument<int_t>("e_splash_copyright_text_size", "Engine::Splash::copyright_text_size", 14);
    //     ConfigManager::load_string_argument<int_t>("e_splash_game_name_text_size", "Engine::Splash::game_name_text_size", 32);
    // }

    // // Initialize config
    // static ConfigsPreInitializeController pre_initialize_configs(on_pre_init, "EngineConfig");
    // static ConfigsInitializeController initialize_configs(on_init, "EngineConfig");

}// namespace Engine
