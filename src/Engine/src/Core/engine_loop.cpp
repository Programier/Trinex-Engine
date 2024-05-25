#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_viewport.hpp>
#include <Core/arguments.hpp>
#include <Core/constants.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/script_engine.hpp>
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


    static void load_external_system_libraries()
    {
        for (const String& library : engine_config.external_system_libraries)
        {
            Library().load(library);
        }
    }


    static void create_window()
    {
        if (rhi == nullptr)
        {
            throw EngineException("Cannot create window without API!");
        }

        global_window_config.update();
        global_window_config.update_using_args();


        WindowManager::create_instance();
        EventSystem::new_system<EventSystem>();

        String client = std::move(global_window_config.client);
        WindowManager::instance()->create_window(global_window_config, nullptr);
        global_window_config.client = std::move(client);
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
        if (!engine_config.api.empty())
        {
            Library api_library(engine_config.api.c_str());
            info_log("Engine", "Using API: %s", engine_config.api.c_str());

            if (!api_library.has_lib())
            {
                throw EngineException("Failed to load API library!");
            }

            // Try to load api loader
            RHI* (*loader)() = api_library.get<RHI*>(Constants::library_load_function_name);

            if (!loader)
            {
                throw EngineException("Failed to get API loader!");
            }

            // Initialize API
            rhi = loader();

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


        VFS::RootFS::create_instance(Platform::find_root_directory(argc, argv));
        engine_config.init();

        // Load libraries
        {
            const Arguments::Argument* argument = arguments.find("libs");
            if (argument && argument->type == Arguments::Type::Array)
            {
                for (const String& library : argument->data.cast<const Arguments::ArrayType&>())
                {
                    Library().load(library);
                }
            }
        }

        create_threads();


        InitializeController().execute();

        load_external_system_libraries();

        Class* engine_class = Class::static_find(engine_config.engine_class, true);
        Object* object      = engine_class->create_object();

        if (object)
        {
            engine_instance = object->instance_cast<BaseEngine>();
            if(engine_instance)
            {
                engine_instance->flags(Object::IsAvailableForGC, false);
            }
        }

        ScriptEngine::initialize();
        ClassInitializeController().execute();
        ScriptEngine::instance()->load_scripts();

        init_api();
        extern void load_default_resources();
        load_default_resources();

        PostInitializeController().execute();

        return 0;
    }

    int_t EngineLoop::init()
    {
        engine_instance->init();
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
        engine_instance->terminate();

        if (rhi)
        {
            call_in_render_thread([]() { rhi->wait_idle(); });
            render_thread()->wait_all();
        }

        GarbageCollector::destroy_all_objects();
        DestroyController().execute();

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
