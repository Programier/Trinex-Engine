#include <Engine/settings.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Trinex::Settings
{
	ENGINE_EXPORT String engine_class        = "Trinex::BaseEngine";
	ENGINE_EXPORT String default_language    = "eng";
	ENGINE_EXPORT String current_language    = "eng";
	ENGINE_EXPORT u32 num_threads            = 0;
	ENGINE_EXPORT i32 lz4_compression_level  = 0;
	ENGINE_EXPORT i32 gc_max_object_per_tick = 1;
	ENGINE_EXPORT i32 fps_limit              = 60;
	ENGINE_EXPORT float screen_percentage    = 1.f;
	ENGINE_EXPORT Vector<String> languages   = {"eng"};
	ENGINE_EXPORT Vector<String> systems;
	ENGINE_EXPORT Vector<String> plugins;
	ENGINE_EXPORT bool debug_shaders = false;

	namespace Rendering
	{
#if PLATFORM_WINDOWS
		ENGINE_EXPORT String rhi = "D3D12";
#else
		ENGINE_EXPORT String rhi = "Vulkan";
#endif
		ENGINE_EXPORT bool force_keep_cpu_resources = false;
		ENGINE_EXPORT u32 shadow_map_size           = 1024;
		ENGINE_EXPORT float anisotropy              = 8.f;
	}// namespace Rendering

	namespace Window
	{
		ENGINE_EXPORT String title                       = "Trinex Engine";
		ENGINE_EXPORT String client                      = "Trinex::DefaultClient";
		ENGINE_EXPORT i32 size_x                         = 1280;
		ENGINE_EXPORT i32 size_y                         = 720;
		ENGINE_EXPORT i32 pos_x                          = -1;
		ENGINE_EXPORT i32 pos_y                          = -1;
		ENGINE_EXPORT bool vsync                         = true;
		ENGINE_EXPORT Vector<WindowAttribute> attributes = {WindowAttribute::Resizable};
		ENGINE_EXPORT Vector<Orientation> orientations;
	}// namespace Window

	namespace Splash
	{
		ENGINE_EXPORT bool show               = true;
		ENGINE_EXPORT String image            = "resources/splash/splash.png";
		ENGINE_EXPORT String font             = "";
		ENGINE_EXPORT i32 startup_text_size   = 14;
		ENGINE_EXPORT i32 version_text_size   = 14;
		ENGINE_EXPORT i32 copyright_text_size = 14;
		ENGINE_EXPORT i32 game_name_text_size = 32;
	}// namespace Splash


	trinex_on_pre_init({.after = {"Trinex::ScriptVector", "Trinex::DefaultScriptAddons"}})
	{
		// LifeCycle::execute(LifeCycle::ReflectionInit, "Trinex::WindowAttribute");
		// LifeCycle::execute(LifeCycle::ReflectionInit, "Trinex::Orientation");

#define bind_value(type, name) e.register_property(#type " " #name, &name)

		auto& e = ScriptEngine::instance();
		e.begin_config_group("engine/engine.config");

		{
			ScriptNamespaceScopedChanger changer("Trinex::Settings");

			bind_value(string, engine_class);
			bind_value(string, default_language);
			bind_value(string, current_language);
			bind_value(uint, num_threads);
			bind_value(int, lz4_compression_level);
			bind_value(int, gc_max_object_per_tick);
			bind_value(float, fps_limit);
			bind_value(Trinex::Vector<string>, languages);
			bind_value(Trinex::Vector<string>, systems);
			bind_value(Trinex::Vector<string>, plugins);
			bind_value(Trinex::Vector<string>, debug_shaders);
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
			// bind_value(Trinex::Vector<Trinex::WindowAttribute>, attributes);
			// bind_value(Trinex::Vector<Trinex::Orientation>, orientations);
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

		e.end_config_group();
	}
}// namespace Trinex::Settings
