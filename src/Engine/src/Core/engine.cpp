#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/entry_point.hpp>
#include <Core/executable_object.hpp>
#include <Core/file_manager.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/platform.hpp>
#include <Core/render_thread.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <Engine/world.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <Window/window_manager.hpp>
#include <no_api.hpp>


namespace Engine
{
    FORCE_INLINE std::chrono::high_resolution_clock::time_point current_time_point()
    {
        return std::chrono::high_resolution_clock::now();
    }

    std::chrono::high_resolution_clock::time_point start_time;


    EngineInstance::EngineInstance()
    {
        for (Thread*& thread : _M_threads) thread = nullptr;
    }

    ENGINE_EXPORT const String& EngineInstance::project_name()
    {
        static String name = "Trinex Engine";
        return name;
    }

    ENGINE_EXPORT const String& EngineInstance::project_name(const String& name)
    {
        const_cast<String&>(project_name()) = name;
        return project_name();
    }

    const String& EngineInstance::api_name() const
    {
        return engine_config.api;
    }

    /////////////////// INITIALIZE ENGINE ///////////////////

    bool EngineInstance::is_inited() const
    {
        return _M_flags(IsInited);
    }

    bool EngineInstance::init_api()
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
            _M_rhi = loader();

            if (!_M_rhi)
            {
                throw EngineException("Failed to init API");
            }

            return true;
        }


        _M_rhi = new NoApi();
        return false;
    }

    EngineInstance* EngineInstance::_M_instance   = nullptr;
    ENGINE_EXPORT EngineInstance* engine_instance = nullptr;


    static EntryPoint* try_load_entry_point(const String& name, Class* base_class)
    {
        Class* class_instance = Class::static_find(name);
        if (!class_instance)
        {
            error_log("Engine", "Failed to load entry point '%s'", name.c_str());
            return nullptr;
        }

        if (!class_instance->is_a(base_class))
        {
            error_log("Engine", "Class '%s' does not inherit from class Engine::EntryPoint!", class_instance->name().c_str());
            return nullptr;
        }

        Object* object    = class_instance->create_object();
        EntryPoint* entry = object->instance_cast<EntryPoint>();

        if (!entry)
        {
            error_log("Engine", "Class '%s' is not entry point!", class_instance->name().c_str());
        }

        return entry;
    }

    static EntryPoint* find_entry_point(const Arguments& args)
    {
        Class* entry_point_base_class = Class::static_find("Engine::EntryPoint");
        EntryPoint* entry_point       = nullptr;

        {
            const Arguments::Argument* arg = args.find("entry");
            if (arg && arg->type == Arguments::Type::String)
            {
                entry_point = try_load_entry_point(arg->get<String>(), entry_point_base_class);
            }
        }

        if (!entry_point)
        {
            entry_point = try_load_entry_point(Constants::default_entry_point, entry_point_base_class);
        }

        return entry_point;
    }


    static void load_external_system_libraries()
    {
        for (const String& library : engine_config.external_system_libraries)
        {
            Library().load(library);
        }
    }

    void EngineInstance::initialize_resources()
    {
        if (_M_rhi)
        {
            // Initialize it only if engine has initialized render interface
            create_window();
            create_render_targets();
        }
    }

    static void create_threads()
    {
        Thread::this_thread()->name("Logic");
        engine_instance->create_thread(ThreadType::RenderThread);
    }

    int EngineInstance::start(int argc, char** argv)
    {
        if (is_inited())
        {
            return -1;
        }

        info_log("TrinexEngine", "Start engine!");
        start_time = current_time_point();

        _M_args.init(argc, argv);

        FileManager* root_manager = const_cast<FileManager*>(FileManager::root_file_manager());
        root_manager->work_dir(Platform::find_root_directory(argc, argv));

        create_threads();

        PreInitializeController().execute();
        _M_flags(PreInitTriggered, true);

        // Load libraries
        {
            const Arguments::Argument* argument = _M_args.find("libs");
            if (argument && argument->type == Arguments::Type::Array)
            {
                for (const String& library : argument->data.cast<const Arguments::ArrayType&>())
                {
                    Library().load(library);
                }
            }
        }

        InitializeController().execute();
        _M_flags(InitTriggered, true);
        engine_config.update();
        engine_config.update_using_args();

        EntryPoint* entry_point = find_entry_point(_M_args);
        if (!entry_point)
        {
            error_log("Engine", "Failed to load entry point for engine start!");
            return -1;
        }

        entry_point->load_configs();
        engine_config.update();
        engine_config.update_using_args();


        load_external_system_libraries();
        ScriptEngine::initialize();

        World::new_system<World>()->name("Global World");

        init_api();

        PostInitializeController().execute();
        _M_flags(PostInitTriggered, true);
        ScriptEngine::instance()->load_scripts();

        initialize_resources();
        _M_flags(IsInited, true);
        int_t status = entry_point->execute(argc - 1, argv + 1);

        if (status == 0)
        {
            info_log("EngineInstance", "Commandlet execution success!");
        }
        else
        {
            error_log("EngineInstance", "Failed to execute commandlet. Error code: %d", status);
        }

        return status;
    }

    RHI* EngineInstance::rhi() const
    {
        return _M_rhi;
    }

    bool EngineInstance::is_shuting_down() const
    {
        return _M_flags(IsShutingDown);
    }

    bool EngineInstance::is_requesting_exit() const
    {
        return _M_flags(IsRequestingExit);
    }

    EngineInstance& EngineInstance::request_exit()
    {
        _M_flags(IsRequestingExit, true);
        return *this;
    }

    const Arguments& EngineInstance::args() const
    {
        return _M_args;
    }

    Arguments& EngineInstance::args()
    {
        return _M_args;
    }

    const Flags& EngineInstance::flags() const
    {
        return _M_flags;
    }


    class DestroyRHI_Task : public ExecutableObject
    {
    private:
        RHI* _M_rhi;

    public:
        DestroyRHI_Task(RHI* rhi) : _M_rhi(rhi)
        {}

        int_t execute() override
        {
            delete _M_rhi;
            return sizeof(DestroyRHI_Task);
        }
    };

    EngineInstance::~EngineInstance()
    {
        request_exit();
        _M_flags(IsShutingDown, true);
        info_log("EngineInstance", "Terminate Engine");

        EngineSystem* engine_system = EngineSystem::instance();
        if (engine_system)
            engine_system->shutdown();

        if (_M_rhi)
        {
            call_in_render_thread([this]() { _M_rhi->wait_idle(); });

            thread(ThreadType::RenderThread)->wait_all();
        }

        Object::collect_garbage(GCFlag::DestroyAll);
        DestroyController().execute();

        for (Thread*& thread : _M_threads)
        {
            if (thread)
                thread->wait_all();
        }


        if (WindowManager::instance())
            WindowManager::instance()->destroy_window(WindowManager::instance()->main_window());

        if (_M_rhi)
        {
            // Cannot delete rhi in logic thread, becouse the gpu resources can be used now
            // So, delete it on render thread
            render_thread()->insert_new_task<DestroyRHI_Task>(_M_rhi);
            render_thread()->wait_all();
            _M_rhi = nullptr;
        }

        if (WindowManager::instance())
            delete WindowManager::instance();

        _M_rhi = nullptr;
        _M_flags(IsInited, false);
        Library::close_all();

        PostDestroyController().execute();

        for (Thread*& thread : _M_threads)
        {
            if (thread && thread->is_destroyable())
            {
                debug_log("Engine", "Destroy thread %s", thread->name().c_str());
                delete thread;
            }
            thread = nullptr;
        }
    }


    void EngineInstance::create_window()
    {
        if (_M_rhi == nullptr)
        {
            throw EngineException("Cannot create window without API!");
        }

        global_window_config.update();
        global_window_config.update_using_args();


        WindowManager::create_instance();
        EventSystem::new_system<EventSystem>();

        WindowManager::instance()->create_window(global_window_config, nullptr);
        AfterRHIInitializeController().execute();
    }

    void EngineInstance::create_render_targets()
    {
        GBuffer::create_instance();
        SceneColorOutput::create_instance();
        thread(ThreadType::RenderThread)->wait_all();
    }

    static const char* thread_name(ThreadType type)
    {
        switch (type)
        {
            case ThreadType::RenderThread:
                return "Render";
            default:
                return "Undefined Thread";
        }
    }

    Thread* EngineInstance::create_thread(ThreadType type)
    {
        Index index = static_cast<Index>(type);

        Thread*& thread = _M_threads[index];
        if (thread == nullptr)
        {
            thread = new Thread(thread_name(type));
        }
        return thread;
    }

    Thread* EngineInstance::thread(ThreadType type) const
    {
        Index index = static_cast<Index>(type);
        return _M_threads[index];
    }

    float EngineInstance::time_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::duration<float>>(current_time_point() - start_time).count();
    }

    Index EngineInstance::frame_index() const
    {
        return _M_frame_index;
    }

    ENGINE_EXPORT Thread* render_thread()
    {
        static Thread* thread = nullptr;
        if (thread == nullptr)
            thread = engine_instance->thread(ThreadType::RenderThread);
        return thread;
    }

    ENGINE_EXPORT bool is_in_render_thread()
    {
        return Thread::this_thread() == render_thread();
    }

    ENGINE_EXPORT int EngineInstance::initialize(int argc, char** argv)
    {
        engine_instance = EngineInstance::create_instance();

        if (!engine_instance->is_inited())
        {
            int result = 0;
            try
            {
                result = engine_instance->start(argc, argv);
            }
            catch (const std::exception& e)
            {
                error_log("TrinexEngine", "%s", e.what());
                result = -1;
            }

            logger->log("Engine", "Begin destroy!");
            delete engine_instance;
            engine_instance = nullptr;

            if (result == 0)
            {
                info_log("Engine", "Execution success. Exit code %d", result);
            }
            else
            {
                error_log("Engine", "Execution fail. Exit code %d", result);
            }
            return result;
        }

        return -1;
    }


}// namespace Engine

ENGINE_EXPORT int main(int argc, char** argv)
{
    return Engine::EngineInstance::initialize(argc, argv);
}
