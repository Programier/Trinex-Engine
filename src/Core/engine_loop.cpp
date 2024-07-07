#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/config_manager.hpp>
#include <Core/constants.hpp>
#include <Core/engine_loop.hpp>
#include <Core/entry_point.hpp>
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

namespace Engine
{
    EngineLoop::EngineLoop()
    {}

    EngineLoop::~EngineLoop()
    {}


    static void init_api(bool force_no_api = false)
    {
        if (Settings::e_api.empty() || force_no_api)
        {
            Settings::e_api         = "None";
            Settings::e_show_splash = false;
        }

        String api = Settings::e_api;
        rhi        = reinterpret_cast<RHI*>(
                Struct::static_find(Strings::format("Engine::RHI::{}", Strings::to_upper(api)), true)->create_struct());
        if (!rhi)
        {
            throw EngineException("Failed to init API");
        }
    }

    static void create_main_window()
    {
        WindowConfig config;
        config.attributes.insert(WindowAttribute::Hidden);
        WindowManager::create_instance();
        EventSystem::new_system<EventSystem>();
        WindowManager::instance()->create_window(config, nullptr)->hide();

        SceneRenderTargets::create_instance();
    }

    static void initialize_filesystem()
    {
        auto vfs                       = VFS::RootFS::create_instance();
        VFS::NativeFileSystem* exec_fs = new VFS::NativeFileSystem(Platform::find_exec_directory());

        vfs->mount("[ExecDir]:", exec_fs, [](VFS::FileSystem* system) { delete system; });
        vfs->mount("", exec_fs);// Temporary root directory is mounted to exec directory

        Platform::bind_platform_mount_points();
    }

    static int_t execute_entry(const StringView& entry_name)
    {
        init_api(true);

        int_t result = 0;
        if (Object* entry_object = Class::static_find(entry_name, true)->create_object())
        {
            if (EntryPoint* entry = entry_object->instance_cast<EntryPoint>())
            {
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

        PreInitializeController().execute();
        initialize_filesystem();
        Project::initialize();

        ReflectionInitializeController().execute();
        ConfigsPreInitializeController().execute();
        ConfigManager::initialize();

        // Load libraries
        {
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


        auto entry_param = Arguments::find("e_entry");
        if (entry_param && entry_param->type == Arguments::Type::String)
        {
            return execute_entry(entry_param->get<const String&>());
        }

        init_api();
        create_main_window();

        return 0;
    }

    int_t EngineLoop::init(int_t argc, const char** argv)
    {
        {
            int result = preinit(argc, argv);
            if (result != 0 || engine_instance->is_requesting_exit())
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
