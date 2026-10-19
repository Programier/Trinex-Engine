#include <Engine/settings.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Trinex::Settings
{
	trinex_api String engine_class        = "Trinex::Engine";
	trinex_api String default_language    = "eng";
	trinex_api String current_language    = "eng";
	trinex_api u32 num_threads            = 0;
	trinex_api i32 lz4_compression_level  = 0;
	trinex_api i32 gc_max_object_per_tick = 1;
	trinex_api i32 fps_limit              = 60;
	trinex_api float screen_percentage    = 1.f;
	trinex_api Array<String> languages   = {"eng"};
	trinex_api Array<String> systems;
	trinex_api Array<String> plugins;
	trinex_api bool debug_shaders = false;

	namespace Rendering
	{
#if PLATFORM_WINDOWS
		trinex_api String rhi = "D3D12";
#else
		trinex_api String rhi = "Vulkan";
#endif
		trinex_api bool force_keep_cpu_resources = false;
		trinex_api u32 shadow_map_size           = 1024;
		trinex_api float anisotropy              = 8.f;
	}// namespace Rendering

	namespace Window
	{
		trinex_api String title                       = "Trinex Engine";
		trinex_api String client                      = "Trinex::DefaultClient";
		trinex_api i32 size_x                         = 1280;
		trinex_api i32 size_y                         = 720;
		trinex_api i32 pos_x                          = -1;
		trinex_api i32 pos_y                          = -1;
		trinex_api bool vsync                         = true;
		trinex_api Array<WindowAttribute> attributes = {WindowAttribute::Resizable};
		trinex_api Array<Orientation> orientations;
	}// namespace Window

	namespace Splash
	{
		trinex_api bool show               = true;
		trinex_api String image            = "resources/splash/splash.png";
		trinex_api String font             = "";
		trinex_api i32 startup_text_size   = 14;
		trinex_api i32 version_text_size   = 14;
		trinex_api i32 copyright_text_size = 14;
		trinex_api i32 game_name_text_size = 32;
	}// namespace Splash


	trinex_on_pre_init({.after = {"Trinex::ScriptVector", "Trinex::DefaultScriptAddons"}})
	{
		// LifeCycle::execute(LifeCycle::ReflectionInit, "Trinex::WindowAttribute");
		// LifeCycle::execute(LifeCycle::ReflectionInit, "Trinex::Orientation");

#define bind_value(type, name) ScriptEngine::register_property(#type " " #name, &name)

		ScriptEngine::begin_config_group("engine/engine.config");

		{
			ScriptNamespaceScopedChanger changer("Trinex::Settings");

			bind_value(string, engine_class);
			bind_value(string, default_language);
			bind_value(string, current_language);
			bind_value(uint, num_threads);
			bind_value(int, lz4_compression_level);
			bind_value(int, gc_max_object_per_tick);
			bind_value(float, fps_limit);
			bind_value(Trinex::Array<string>, languages);
			bind_value(Trinex::Array<string>, systems);
			bind_value(Trinex::Array<string>, plugins);
			bind_value(Trinex::Array<string>, debug_shaders);
		}

		{
			ScriptNamespaceScopedChanger changer("Trinex::Settings::Rendering");

			using namespace Rendering;

			bind_value(string, rhi);
			bind_value(uint, shadow_map_size);
		}

		{
			ScriptNamespaceScopedChanger changer("Trinex::Settings::Window");

			using namespace Window;

			bind_value(string, title);
			bind_value(string, client);
			bind_value(int, size_x);
			bind_value(int, size_y);
			bind_value(int, pos_x);
			bind_value(int, pos_y);
			bind_value(bool, vsync);
			// bind_value(Trinex::Array<Trinex::WindowAttribute>, attributes);
			// bind_value(Trinex::Array<Trinex::Orientation>, orientations);
		}

		{
			ScriptNamespaceScopedChanger changer("Trinex::Settings::Splash");
			using namespace Splash;

			bind_value(bool, show);
			bind_value(string, image);
			bind_value(string, font);
			bind_value(int, startup_text_size);
			bind_value(int, version_text_size);
			bind_value(int, copyright_text_size);
			bind_value(int, game_name_text_size);
		}

		ScriptEngine::end_config_group();
	}
}// namespace Trinex::Settings
