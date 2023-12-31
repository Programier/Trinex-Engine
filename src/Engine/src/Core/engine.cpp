#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/executable_object.hpp>
#include <Core/file_manager.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/render_thread_call.hpp>
#include <Core/string_functions.hpp>
#include <Core/system.hpp>
#include <Core/thread.hpp>
#include <Engine/world.hpp>
#include <Graphics/renderer.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Systems/engine_system.hpp>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <Window/window_manager.hpp>
#include <cstring>
#include <glm/gtc/quaternion.hpp>
#include <no_api.hpp>


namespace Engine
{
    extern void trinex_init_sdl();
    extern void trinex_terminate_sdl();

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


    SystemName EngineInstance::system_type() const
    {
#if PLATFORM_WINDOWS
        return SystemName::WindowsOS;
#elif PLATFORM_ANDROID
        return SystemName::AndroidOS;
#else
        return SystemName::LinuxOS;
#endif
    }

    EngineAPI EngineInstance::api() const
    {
        return _M_api;
    }

    const String& EngineInstance::api_name() const
    {
        return engine_config.api;
    }

    /////////////////// INITIALIZE ENGINE ///////////////////

    bool EngineInstance::is_inited() const
    {
        return _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsInited)];
    }

    static EngineAPI get_api_by_name(const String& name)
    {
        if (name == "Vulkan")
            return EngineAPI::Vulkan;
        else if (name == "OpenGLES")
            return EngineAPI::OpenGLES;
        else if (name == "OpenGL")
            return EngineAPI::OpenGL;

        return EngineAPI::NoAPI;
    }

    EngineInstance& EngineInstance::init_api()
    {
        if (_M_api != EngineAPI::NoAPI)
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
        }
        else
        {
            _M_rhi = new NoApi();
        }
        return *this;
    }

    EngineInstance* EngineInstance::_M_instance   = nullptr;
    ENGINE_EXPORT EngineInstance* engine_instance = nullptr;


    static CommandLet* try_load_commandlet(const String& name, Class* base_class)
    {
        Class* class_instance = Class::static_find_class(name);
        if (!class_instance)
        {
            error_log("Engine", "Failed to load commandlet '%s'", name.c_str());
            return nullptr;
        }

        if (!class_instance->contains_class(base_class))
        {
            error_log("Engine", "Class '%s' does not inherit from class Engine::CommandLet!", class_instance->name().c_str());
            return nullptr;
        }

        Object* object         = class_instance->create_object();
        CommandLet* commandlet = object->instance_cast<CommandLet>();

        if (!commandlet)
        {
            error_log("Engine", "Class '%s' is not commandlet!", class_instance->name().c_str());
        }

        return commandlet;
    }

    static CommandLet* find_commandlet(int argc, char** argv)
    {
        Class* commandlet_base_class = Class::static_find_class("Engine::CommandLet");
        CommandLet* commandlet       = nullptr;
        // Load commandlet
        if (argc > 1)
        {
            commandlet = try_load_commandlet(argv[1], commandlet_base_class);
        }

        if (!commandlet)
        {
            commandlet = try_load_commandlet(Constants::default_commandlet, commandlet_base_class);
        }

        return commandlet;
    }


    static void load_external_system_libraries()
    {
        for (const String& library : engine_config.external_system_libraries)
        {
            Library().load(library);
        }
    }

    void EngineInstance::init_engine_for_rendering()
    {
        create_thread(ThreadType::RenderThread);
        create_window();
        create_render_targets();
    }

    int EngineInstance::start(int argc, char** argv)
    {
        if (is_inited())
        {
            return -1;
        }


        info_log("TrinexEngine", "Start engine!");
        start_time = current_time_point();


        FileManager* root_manager = const_cast<FileManager*>(FileManager::root_file_manager());

#if PLATFORM_ANDROID
        root_manager->work_dir(Strings::format("/sdcard/TrinexGames/{}/", EngineInstance::project_name()));
#else
        if (argc > 0)
        {
            root_manager->work_dir(FileManager::dirname_of(argv[0]));
        }
#endif

        Thread::this_thread()->name("Logic");

        PreInitializeController().execute();
        engine_config.update();

        ScriptEngine::instance();
        InitializeController().execute();

        load_external_system_libraries();

        ScriptEngine::instance()->load_scripts();

        CommandLet* commandlet = find_commandlet(argc, argv);

        if (commandlet)
        {
            commandlet->load_configs();
        }

        engine_config.update();


        load_external_system_libraries();
        info_log("EngineInstance", "Work dir is '%s'", root_manager->work_dir().c_str());
        World::new_system<World>()->name("Global World");

        _M_api = get_api_by_name(engine_config.api);

        init_api();
        _M_renderer = new Renderer(_M_rhi);

        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsInited)] = true;

        if (engine_config.max_g_buffer_height < 200)
        {
            engine_config.max_g_buffer_height = static_cast<uint_t>(Monitor::height());
        }

        if (engine_config.max_g_buffer_width < 200)
        {
            engine_config.max_g_buffer_width = static_cast<uint_t>(Monitor::width());
        }

        // If API is not NoApi, than we need to init Window
        if (_M_api != EngineAPI::NoAPI)
        {
            init_engine_for_rendering();
        }

        Object::mark_internal_objects();
        int_t status = commandlet ? commandlet->execute(argc - 1, argv + 1) : launch();

        if (status == 0)
        {
            info_log("EngineInstance", "Commandlet execution success!");
        }
        else
        {
            info_log("EngineInstance", "Failed to execute commandlet. Error code: %d", status);
        }

        return status;
    }

    RHI* EngineInstance::rhi() const
    {
        return _M_rhi;
    }

    class Renderer* EngineInstance::renderer() const
    {
        return _M_renderer;
    }

    bool EngineInstance::is_shuting_down() const
    {
        return _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsShutingDown)];
    }

    bool EngineInstance::is_requesting_exit() const
    {
        return _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsRequestingExit)];
    }

    EngineInstance& EngineInstance::request_exit()
    {
        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsRequestingExit)] = true;
        return *this;
    }

    EngineInstance& EngineInstance::trigger_terminate_functions()
    {
        DestroyController().execute();
        return *this;
    }

    EngineInstance::~EngineInstance()
    {
        request_exit();
        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsShutingDown)] = true;
        info_log("EngineInstance", "Terminate Engine");

        if (_M_rhi)
        {
            call_in_render_thread([this]() { _M_rhi->wait_idle(); });

            thread(ThreadType::RenderThread)->wait_all();
        }

        engine_instance->trigger_terminate_functions();
        Object::collect_garbage(GCFlag::DestroyAll);

        for (Thread*& thread : _M_threads)
        {
            if (thread)
                thread->wait_all();
        }

        if (_M_renderer)
            delete _M_renderer;

        if (WindowManager::instance())
            delete WindowManager::instance();

        if (_M_rhi)
            delete _M_rhi;

        _M_rhi                                                              = nullptr;
        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsInited)] = false;
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
        global_window_config.api_name = engine_config.api;

        WindowManager::create_instance();

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
            return result;
        }

        return -1;
    }


}// namespace Engine

ENGINE_EXPORT int main(int argc, char** argv)
{
    return Engine::EngineInstance::initialize(argc, argv);
}
