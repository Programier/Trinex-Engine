#include <Core/enums.hpp>
#include <Core/etl/script_array.hpp>


namespace Engine::Settings
{
    extern ENGINE_EXPORT String e_engine;
    extern ENGINE_EXPORT String e_api;
    extern ENGINE_EXPORT String e_default_language;
    extern ENGINE_EXPORT String e_current_language;
    extern ENGINE_EXPORT int_t e_lz4_compression_level;
    extern ENGINE_EXPORT int_t e_gc_max_object_per_tick;
    extern ENGINE_EXPORT int_t e_fps_limit;
    extern ENGINE_EXPORT float e_screen_percentage;
    extern ENGINE_EXPORT ScriptArray<String, "string"> e_languages;
    extern ENGINE_EXPORT ScriptArray<String, "string"> e_systems;
    extern ENGINE_EXPORT ScriptArray<String, "string"> e_libs;

    extern ENGINE_EXPORT String w_title;
    extern ENGINE_EXPORT String w_client;
    extern ENGINE_EXPORT int_t w_size_x;
    extern ENGINE_EXPORT int_t w_size_y;
    extern ENGINE_EXPORT int_t w_pos_x;
    extern ENGINE_EXPORT int_t w_pos_y;
    extern ENGINE_EXPORT bool w_vsync;
    extern ENGINE_EXPORT ScriptArray<WindowAttribute, "Engine::WindowAttribute"> w_attributes;
    extern ENGINE_EXPORT ScriptArray<Orientation, "Engine::Orientation"> w_orientations;

    extern ENGINE_EXPORT bool e_show_splash;
    extern ENGINE_EXPORT String e_splash_image;
    extern ENGINE_EXPORT String e_splash_font;
    extern ENGINE_EXPORT int_t e_splash_startup_text_size;
    extern ENGINE_EXPORT int_t e_splash_version_text_size;
    extern ENGINE_EXPORT int_t e_splash_copyright_text_size;
    extern ENGINE_EXPORT int_t e_splash_game_name_text_size;
}// namespace Engine::Settings
