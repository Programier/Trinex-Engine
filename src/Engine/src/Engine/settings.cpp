#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/settings.hpp>
#include <scriptarray.h>

namespace Engine::Settings
{
    ENGINE_EXPORT String e_engine;
    ENGINE_EXPORT String e_api;
    ENGINE_EXPORT String e_default_language;
    ENGINE_EXPORT String e_current_language;
    ENGINE_EXPORT int_t e_lz4_compression_level;
    ENGINE_EXPORT int_t e_gc_max_object_per_tick;
    ENGINE_EXPORT int_t e_fps_limit;
    ENGINE_EXPORT ScriptArray<String, "string"> e_languages;
    ENGINE_EXPORT ScriptArray<String, "string"> e_systems;
    ENGINE_EXPORT ScriptArray<String, "string"> e_libs;

    ENGINE_EXPORT String w_title;
    ENGINE_EXPORT String w_client;
    ENGINE_EXPORT int_t w_size_x;
    ENGINE_EXPORT int_t w_size_y;
    ENGINE_EXPORT int_t w_pos_x;
    ENGINE_EXPORT int_t w_pos_y;
    ENGINE_EXPORT bool w_vsync;
    ENGINE_EXPORT ScriptArray<WindowAttribute, "Engine::WindowAttribute"> w_attributes;
    ENGINE_EXPORT ScriptArray<WindowOrientation, "Engine::WindowOrientation"> w_orientations;

    ENGINE_EXPORT bool e_show_splash;
    ENGINE_EXPORT String e_splash_image;
    ENGINE_EXPORT String e_splash_font;
    ENGINE_EXPORT int_t e_splash_startup_text_size;
    ENGINE_EXPORT int_t e_splash_version_text_size;
    ENGINE_EXPORT int_t e_splash_copyright_text_size;
    ENGINE_EXPORT int_t e_splash_game_name_text_size;

    static void init()
    {
#define bind_value(name, group) ConfigManager::register_property("Engine::Settings::" #name, name, #group)

        bind_value(e_engine, engine);
        bind_value(e_api, engine);
        bind_value(e_default_language, engine);
        bind_value(e_current_language, engine);
        bind_value(e_lz4_compression_level, engine);
        bind_value(e_gc_max_object_per_tick, engine);
        bind_value(e_fps_limit, engine);
        bind_value(e_languages, engine);
        bind_value(e_systems, engine);
        bind_value(e_libs, engine);
        bind_value(w_title, engine);
        bind_value(w_client, engine);
        bind_value(w_size_x, engine);
        bind_value(w_size_y, engine);
        bind_value(w_pos_x, engine);
        bind_value(w_pos_y, engine);
        bind_value(w_vsync, engine);

        w_attributes.create();
        w_orientations.create();

        ConfigManager::register_custom_property("Engine::Settings::w_attributes", w_attributes.array(),
                                                w_attributes.full_declaration().c_str(), "engine");
        ConfigManager::register_custom_property("Engine::Settings::w_orientations", w_orientations.array(),
                                                w_orientations.full_declaration().c_str(), "engine");
    }

    static void destroy()
    {
        e_languages.release();
        w_attributes.release();
        w_orientations.release();
    }

    static ReflectionInitializeController on_init(init, "Engine::Settings",
                                                  {"Engine::WindowAttribute", "Engine::WindowOrientation"});
    static DestroyController on_destroy(destroy, "Engine::Settings");
}// namespace Engine::Settings