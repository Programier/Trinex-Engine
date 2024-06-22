#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/config_manager.hpp>
#include <Core/constants.hpp>
#include <Core/filesystem/native_file_system.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
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
        WindowConfig config;
        config.attributes.insert(WindowAttribute::Hidden);
        WindowManager::create_instance();
        EventSystem::new_system<EventSystem>();
        WindowManager::instance()->create_window(config, nullptr)->hide();
    }

    static bool init_api()
    {
        String api = Settings::e_api;
        if (!api.empty())
        {
            rhi = reinterpret_cast<RHI*>(
                    Struct::static_find(Strings::format("Engine::RHI::{}", Strings::to_upper(api)), true)->create_struct());
            if (!rhi)
            {
                throw EngineException("Failed to init API");
            }

            create_window();
            SceneRenderTargets::create_instance();
            return true;
        }


        rhi = new NoApi();
        return false;
    }

    static void initialize_filesystem()
    {
        auto vfs                       = VFS::RootFS::create_instance();
        VFS::NativeFileSystem* exec_fs = new VFS::NativeFileSystem(Platform::find_exec_directory());

        vfs->mount("[ExecDir]:", exec_fs, [](VFS::FileSystem* system) { delete system; });
        vfs->mount("", exec_fs);// Temporary root directory is mounted to exec directory

        Platform::bind_platform_mount_points();
    }

    int_t EngineLoop::preinit(int_t argc, const char** argv)
    {
        info_log("TrinexEngine", "Start engine!");

        Arguments arguments;
        arguments.init(argc, argv);

        PreInitializeController().execute();
        initialize_filesystem();
        Project::initialize();

        ReflectionInitializeController().execute();

        ConfigsPreInitializeController().execute();
        ConfigManager::initialize();


        // Load libraries
        {
            auto begin = Settings::e_libs.begin();
            auto end   = Settings::e_libs.end();

            auto status = begin == end;
            for (const String& library : Settings::e_libs)
            {
                Library().load(library);
            }
        }

        create_threads();

        Class* engine_class = Class::static_find(Settings::e_engine, true);
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
        ScriptEngine::load_scripts();

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

        bool show_splash = Settings::e_show_splash;

        if (show_splash)
        {
            Engine::show_splash_screen();
            Engine::splash_screen_text(Engine::SplashTextType::GameName, Project::name);
            Engine::splash_screen_text(Engine::SplashTextType::VersionInfo, Project::version);
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

        if (rhi)
        {
            // Cannot delete rhi in logic thread, because the gpu resources can be used now
            // So, delete it on render thread
            render_thread()->insert_new_task<DestroyRHI_Task>();
            render_thread()->wait_all();
            rhi = nullptr;
        }

        if (WindowManager::instance())
            WindowManager::instance()->destroy_window(WindowManager::instance()->main_window());

        if (WindowManager::instance())
            delete WindowManager::instance();

        rhi = nullptr;
        Library::close_all();
        PostDestroyController().execute();

        GarbageCollector::destroy(engine_instance);
        engine_instance = nullptr;
    }
}// namespace Engine
