#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loop.hpp>
#include <Core/entry_point.hpp>
#include <Core/etl/templates.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Core/threading/thread.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
#include <Engine/splash_screen.hpp>
#include <Graphics/render_viewport.hpp>
#include <Platform/platform.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	EngineLoop::EngineLoop() {}

	EngineLoop::~EngineLoop() {}

	static void initialize_graphics_api(bool force_no_api = false)
	{
		if (Settings::Rendering::rhi.empty() || force_no_api)
		{
			Settings::Rendering::rhi = "None";
			Settings::Splash::show   = false;
		}

		String api = Settings::Rendering::rhi;
		auto decl  = Strings::format("Engine::TRINEX_RHI::{}", Strings::to_upper(api));
		rhi        = reinterpret_cast<RHI*>(Refl::Struct::static_find(decl, Refl::FindFlags::IsRequired)->create_struct());

		if (!rhi)
		{
			throw EngineException("Failed to init API");
		}
	}

	static void initialize_filesystem()
	{
		auto vfs      = VFS::RootFS::create_instance();
		auto exec_dir = Platform::find_exec_directory();

		vfs->mount("[exec_dir]:", exec_dir, VFS::FileSystem::Native);
		vfs->mount("[assets_dir]:/TrinexEngine", "[exec_dir]:/resources/TrinexEngine/assets");
		vfs->mount("[shaders_dir]:/TrinexEngine", "[exec_dir]:/resources/TrinexEngine/shaders");
	}

	static int_t execute_entry(const StringView& entry_name)
	{
		initialize_graphics_api(true);

		int_t result = 0;
		if (Object* entry_object = Refl::Class::static_find(entry_name, Refl::FindFlags::IsRequired)->create_object())
		{
			if (EntryPoint* entry = entry_object->instance_cast<EntryPoint>())
			{
				Settings::Rendering::force_keep_cpu_resources = true;

				result = entry->execute();
			}
			else
			{
				throw EngineException("Failed to cast entry point!");
			}
		}
		else
		{
			throw EngineException("Failed to create entry point!");
		}
		engine_instance->request_exit();
		return result;
	}

	int_t EngineLoop::preinit(int_t argc, const char** argv)
	{
		info_log("TrinexEngine", "Start engine!");

		Arguments arguments;
		arguments.init(argc, argv);
		initialize_filesystem();
		ScriptEngine::initialize();

		PreInitializeController().execute();
		Project::initialize();

		ConfigManager::initialize();

		// Load libraries
		{
			for (const String& plugin : Settings::plugins)
			{
				Library().load(plugin);
			}
		}

		ReflectionInitializeController().execute();

		Refl::Class* engine_class = Refl::Class::static_find(Settings::engine_class, Refl::FindFlags::IsRequired);
		engine_instance           = Object::instance_cast<BaseEngine>(engine_class->create_object());

		if (engine_instance == nullptr)
		{
			error_log("EngineLoop", "Failed to create engine instance!");
			return -1;
		}

		ScriptEngine::load_scripts();

		auto entry_param = Arguments::find("entry");
		if (entry_param && entry_param->type == Arguments::Type::String)
		{
			return execute_entry(entry_param->get<const String&>());
		}

		System::system_of<EngineSystem>();

		initialize_graphics_api();

		// Setup main window
		WindowConfig config;
		config.attributes.insert(WindowAttribute::Hidden);
		WindowManager::create_instance()->create_window(config, nullptr)->hide();
		return 0;
	}

	void EngineLoop::init(int_t argc, const char** argv)
	{
		create_threads();

		{
			int result = preinit(argc, argv);
			if (result != 0 || engine_instance->is_requesting_exit())
				return;
		}

		const bool show_splash = Settings::Splash::show;

		float wait_time = engine_instance->time_seconds();
		engine_instance->init();

		extern void load_default_resources();
		load_default_resources();

		if (show_splash)
		{
			Engine::show_splash_screen();
			Engine::splash_screen_text(Engine::SplashTextType::GameName, Project::name);
			Engine::splash_screen_text(Engine::SplashTextType::VersionInfo, Project::version);
			Engine::splash_screen_text(Engine::SplashTextType::StartupProgress, "Starting Engine");
		}

		StartupResourcesInitializeController().execute();
		InitializeController().execute();

		wait_time = 2.0f - (engine_instance->time_seconds() - wait_time);

		if (wait_time > 0.f)
		{
			Thread::static_sleep_for(wait_time);
		}

		if (show_splash)
			Engine::hide_splash_screen();

		if (Window* window = WindowManager::instance()->main_window())
		{
			window->show();
		}

		engine_instance->make_inited();
	}

	void EngineLoop::update()
	{
		engine_instance->update();
	}

	void EngineLoop::terminate()
	{
		info_log("EngineInstance", "Terminate Engine");

		DestroyController().execute();
		engine_instance->terminate();

		if (WindowManager::instance())
			WindowManager::instance()->destroy_window(WindowManager::instance()->main_window());

		if (WindowManager::instance())
			trx_delete WindowManager::instance();

		GarbageCollector::destroy_all_objects();

		if (rhi)
		{
			rhi->info.struct_instance->destroy_struct(rhi);
			rhi = nullptr;
		}

		Library::close_all();

		GarbageCollector::destroy(engine_instance);
		engine_instance = nullptr;

		PostDestroyController().execute();
		destroy_threads();
	}
}// namespace Engine
