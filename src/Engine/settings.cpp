#include <Core/engine_loading_controllers.hpp>
#include <Engine/settings.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine::Settings
{
	ENGINE_EXPORT String engine_class          = "Engine::BaseEngine";
	ENGINE_EXPORT String default_language      = "eng";
	ENGINE_EXPORT String current_language      = "eng";
	ENGINE_EXPORT uint_t num_threads           = 0;
	ENGINE_EXPORT int_t lz4_compression_level  = 0;
	ENGINE_EXPORT int_t gc_max_object_per_tick = 1;
	ENGINE_EXPORT int_t fps_limit              = 60;
	ENGINE_EXPORT float screen_percentage      = 1.f;
	ENGINE_EXPORT Vector<String> languages     = {"eng"};
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
		ENGINE_EXPORT uint_t shadow_map_size        = 1024;
		ENGINE_EXPORT bool enable_hdr               = true;
		ENGINE_EXPORT float anisotropy              = 8.f;
	}// namespace Rendering

	namespace Window
	{
		ENGINE_EXPORT String title = "Trinex Engine";
		ENGINE_EXPORT String client;
		ENGINE_EXPORT int_t size_x                       = 1280;
		ENGINE_EXPORT int_t size_y                       = 720;
		ENGINE_EXPORT int_t pos_x                        = -1;
		ENGINE_EXPORT int_t pos_y                        = -1;
		ENGINE_EXPORT bool vsync                         = true;
		ENGINE_EXPORT Vector<WindowAttribute> attributes = {WindowAttribute::Resizable};
		ENGINE_EXPORT Vector<Orientation> orientations;
	}// namespace Window

	namespace Splash
	{
		ENGINE_EXPORT bool show                 = true;
		ENGINE_EXPORT String image              = "resources/splash/splash.png";
		ENGINE_EXPORT String font               = "";
		ENGINE_EXPORT int_t startup_text_size   = 14;
		ENGINE_EXPORT int_t version_text_size   = 14;
		ENGINE_EXPORT int_t copyright_text_size = 14;
		ENGINE_EXPORT int_t game_name_text_size = 32;
	}// namespace Splash


	static void init()
	{
		ReflectionInitializeController().require("Engine::WindowAttribute").require("Engine::Orientation");

#define bind_value(type, name) e.register_property(#type " " #name, &name)

		auto& e = ScriptEngine::instance();
		e.begin_config_group("engine/engine.config");

		{
			ScriptNamespaceScopedChanger changer("Engine::Settings");

			bind_value(string, engine_class);
			bind_value(string, default_language);
			bind_value(string, current_language);
			bind_value(uint, num_threads);
			bind_value(int, lz4_compression_level);
			bind_value(int, gc_max_object_per_tick);
			bind_value(float, fps_limit);
			bind_value(Engine::Vector<string>, languages);
			bind_value(Engine::Vector<string>, systems);
			bind_value(Engine::Vector<string>, plugins);
			bind_value(Engine::Vector<string>, debug_shaders);
		}

		{
			ScriptNamespaceScopedChanger changer("Engine::Settings::Rendering");

			using namespace Rendering;

			bind_value(string, rhi);
			bind_value(uint, shadow_map_size);
			bind_value(bool, enable_hdr);
		}

		{
			ScriptNamespaceScopedChanger changer("Engine::Settings::Window");

			using namespace Window;

			bind_value(string, title);
			bind_value(string, client);
			bind_value(int, size_x);
			bind_value(int, size_y);
			bind_value(int, pos_x);
			bind_value(int, pos_y);
			bind_value(bool, vsync);
			bind_value(Engine::Vector<Engine::WindowAttribute>, attributes);
			bind_value(Engine::Vector<Engine::Orientation>, orientations);
		}

		{
			ScriptNamespaceScopedChanger changer("Engine::Settings::Splash");
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

	static PreInitializeController on_init(init, "Engine::Settings");
}// namespace Engine::Settings
