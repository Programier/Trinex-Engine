#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/settings.hpp>

namespace Engine::Settings
{
	ENGINE_EXPORT String e_engine = "Engine::EngineInstance";
	ENGINE_EXPORT String e_api;
	ENGINE_EXPORT String e_default_language;
	ENGINE_EXPORT String e_current_language;
	ENGINE_EXPORT int_t e_lz4_compression_level;
	ENGINE_EXPORT int_t e_gc_max_object_per_tick = 100;
	ENGINE_EXPORT int_t e_fps_limit;

#if PLATFORM_ANDROID
	ENGINE_EXPORT float e_screen_percentage = 1.f;
#else
	ENGINE_EXPORT float e_screen_percentage = 1.f;
#endif
	ENGINE_EXPORT Vector<String> e_languages;
	ENGINE_EXPORT Vector<String> e_systems;
	ENGINE_EXPORT Vector<String> e_libs;

	ENGINE_EXPORT String w_title;
	ENGINE_EXPORT String w_client;
	ENGINE_EXPORT int_t w_size_x = 1280;
	ENGINE_EXPORT int_t w_size_y = 720;
	ENGINE_EXPORT int_t w_pos_x  = -1;
	ENGINE_EXPORT int_t w_pos_y  = -1;
	ENGINE_EXPORT bool w_vsync;
	ENGINE_EXPORT Vector<WindowAttribute> w_attributes;
	ENGINE_EXPORT Vector<Orientation> w_orientations;

	ENGINE_EXPORT bool e_show_splash                 = true;
	ENGINE_EXPORT String e_splash_image              = "resources/splash/splash.png";
	ENGINE_EXPORT String e_splash_font               = "";
	ENGINE_EXPORT int_t e_splash_startup_text_size   = 14;
	ENGINE_EXPORT int_t e_splash_version_text_size   = 14;
	ENGINE_EXPORT int_t e_splash_copyright_text_size = 14;
	ENGINE_EXPORT int_t e_splash_game_name_text_size = 32;


	static void init()
	{
#define bind_value(name, group) ConfigManager::register_property("Engine::Settings::" #name, name, #group)
#define bind_enum(name, group, enum_name) ConfigManager::register_property("Engine::Settings::" #name, name, #group, #enum_name)

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
		bind_enum(w_attributes, engine, Engine::WindowAttribute);
		bind_enum(w_orientations, engine, Engine::Orientation);
		bind_value(e_show_splash, engine);
		bind_value(e_splash_image, engine);
		bind_value(e_splash_font, engine);
		bind_value(e_splash_startup_text_size, engine);
		bind_value(e_splash_version_text_size, engine);
		bind_value(e_splash_copyright_text_size, engine);
		bind_value(e_splash_game_name_text_size, engine);
	}

	static ReflectionInitializeController on_init(init, "Engine::Settings", {"Engine::WindowAttribute", "Engine::Orientation"});
}// namespace Engine::Settings
