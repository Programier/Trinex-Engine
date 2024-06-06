#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/config_manager.hpp>
#include <Core/constants.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Engine/splash_screen.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <no_api.hpp>

#include <Core/engine_loop.hpp>

namespace Engine
{
    EngineLoop::EngineLoop()
    {}

    EngineLoop::~EngineLoop()
    {}

    static void create_window()
    {
        if (rhi == nullptr)
        {
            throw EngineException("Cannot create window without API!");
        }

        WindowConfig config;
        config.attributes.insert(WindowAttribute::Hidden);
        WindowManager::create_instance();
        EventSystem::new_system<EventSystem>();
        WindowManager::instance()->create_window(config, nullptr)->hide();
    }

    static void create_render_targets()
    {
        GBuffer::create_instance();
        SceneColorOutput::create_instance();
        GBufferBaseColorOutput::create_instance();
        render_thread()->wait_all();
    }

    static bool init_api()
    {
        String api = ConfigManager::get_string("Engine::api");
        if (!api.empty())
        {
            rhi = reinterpret_cast<RHI*>(
                    Struct::static_find(Strings::format("Engine::RHI::{}", Strings::to_upper(api)), true)->create_struct());
            if (!rhi)
            {
                throw EngineException("Failed to init API");
            }

            create_window();
            create_render_targets();
            return true;
        }


        rhi = new NoApi();
        return false;
    }

    int_t EngineLoop::preinit(int_t argc, const char** argv)
    {
        info_log("TrinexEngine", "Start engine!");

        Arguments arguments;
        arguments.init(argc, argv);

        PreInitializeController().execute();

        // Initialize script engine and load reflections
        ScriptEngine::initialize();

        ReflectionInitializeController().execute();

        ConfigsPreInitializeController().execute();
        VFS::RootFS::create_instance(Platform::find_root_directory());
        ConfigManager::initialize();


        // Load libraries
        {
            Vector<String> external_system_libraries = ConfigManager::get_string_array("Engine::libs");

            for (const String& library : external_system_libraries)
            {
                Library().load(library);
            }
        }

        create_threads();

        Class* engine_class = Class::static_find(ConfigManager::get_string("Engine::engine"), true);
        Object* object      = engine_class->create_object();

        if (object)
        {
            engine_instance = object->instance_cast<BaseEngine>();
            if (engine_instance)
            {
                engine_instance->flags(Object::IsAvailableForGC, false);
            }
        }

        ReflectionInitializeController().execute();
        ScriptEngine::instance()->load_scripts();

        init_api();
        return 0;
    }

    int_t EngineLoop::init(int_t argc, const char** argv)
    {
        {
            int result = preinit(argc, argv);
            if (result != 0)
                return result;
        }

        bool show_splash = ConfigManager::get_bool("Engine::Splash::show");

        if (show_splash)
        {
            Engine::show_splash_screen();
            Engine::splash_screen_text(Engine::SplashTextType::GameName, ConfigManager::get_string("Engine::project_name"));
            Engine::splash_screen_text(Engine::SplashTextType::VersionInfo, ConfigManager::get_string("Engine::version"));
            Engine::splash_screen_text(Engine::SplashTextType::StartupProgress, "Starting Engine");
        }

        float wait_time = engine_instance->time_seconds();
        engine_instance->init();

        extern void load_default_resources();
        load_default_resources();

        InitializeController().execute();

        wait_time = 2.0f - (engine_instance->time_seconds() - wait_time);

        if (wait_time > 0.f)
        {
            Thread::sleep_for(wait_time);
        }

        if (show_splash)
            Engine::hide_splash_screen();

        if (Window* window = WindowManager::instance()->main_window())
        {
            window->show();
        }

        return 0;
    }

    void EngineLoop::update()
    {
        engine_instance->update();
    }

    class DestroyRHI_Task : public ExecutableObject
    {

    public:
        int_t execute() override
        {
            delete rhi;
            return sizeof(DestroyRHI_Task);
        }
    };

    void EngineLoop::terminate()
    {
        info_log("EngineInstance", "Terminate Engine");
        DestroyController().execute();

        engine_instance->terminate();

        if (rhi)
        {
            call_in_render_thread([]() { rhi->wait_idle(); });
            render_thread()->wait_all();
        }

        GarbageCollector::destroy_all_objects();
        render_thread()->wait_all();


        if (WindowManager::instance())
            WindowManager::instance()->destroy_window(WindowManager::instance()->main_window());

        if (rhi)
        {
            // Cannot delete rhi in logic thread, becouse the gpu resources can be used now
            // So, delete it on render thread
            render_thread()->insert_new_task<DestroyRHI_Task>();
            render_thread()->wait_all();
            rhi = nullptr;
        }

        if (WindowManager::instance())
            delete WindowManager::instance();

        rhi = nullptr;
        Library::close_all();
        PostDestroyController().execute();

        GarbageCollector::destroy(engine_instance);
        engine_instance = nullptr;
    }
}// namespace Engine
