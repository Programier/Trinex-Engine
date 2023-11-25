#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/executable_object.hpp>
#include <Core/file_manager.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Core/system.hpp>
#include <Core/thread.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/renderer.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Systems/engine_system.hpp>
#include <Window/config.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
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

    Window* EngineInstance::window() const
    {
        return _M_window;
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
            error_log("Engine", "Class '%s' does not inherit from class Engine::CommandLet!",
                      class_instance->name().c_str());
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
    }

    int EngineInstance::start(int argc, char** argv)
    {
        if (is_inited())
        {
            return -1;
        }


        info_log("TrinexEngine", "Start engine!");
        start_time = current_time_point();

        PreInitializeController().execute();

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

        ScriptEngine::instance();
        InitializeController().execute();
        engine_config.update();

        CommandLet* commandlet = find_commandlet(argc, argv);

        if (commandlet)
        {
            commandlet->load_configs();
            engine_config.update();
        }


        load_external_system_libraries();
        info_log("EngineInstance", "Work dir is '%s'", root_manager->work_dir().c_str());
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

        int_t status = commandlet ? commandlet->execute(argc - 1, argv + 1) : launch_systems();

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
#if defined(__arm__) || defined(__aarch64__)
    asm(R"(
is_on_stack_asm:
    mov x0, sp
    cmp x0, x1
    cset w0, gt
    ret

stack_address:
    mov w0, #0
    ret
)");

    extern "C" bool is_on_stack_asm(void* ptr);

#elif defined(__x86_64__)
    asm(R"(
is_on_stack_asm:
    movq %rsp, %rax
    cmpq %rdi, %rax
    jbe stack_address
    movl $1, %eax
    ret

stack_address:
    movl $0, %eax
    ret
)");

    extern "C" bool is_on_stack_asm(void* ptr);

#else

#error "Function is_on_stack_asm in not implemented for current arch!"

#endif

    bool EngineInstance::is_on_stack(void* ptr)
    {
        bool result = is_on_stack_asm(ptr);
        return result;
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
            _M_rhi->wait_idle();
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
        if (_M_rhi)
            delete _M_rhi;
        if (_M_window)
            Object::delete_object(_M_window);

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

        String libname = Strings::format("WindowSystem{}", engine_config.window_system);
        Library library(libname);

        if (!library.has_lib())
        {
            throw EngineException("Cannot load window system library!");
        }

        struct CreateWindowTask : public ExecutableObject {
        public:
            WindowInterface* (*_M_loader)();
            WindowInterface* _M_interface = nullptr;

            CreateWindowTask(WindowInterface* (*loader)()) : _M_loader(loader)
            {}

            int_t execute() override
            {
                info_log("Window", "Starting creating window");
                _M_interface = _M_loader();

                if (!_M_interface)
                {
                    throw EngineException("Cannot create window interface!");
                }

                _M_interface->init(global_window_config);
                _M_interface->update_monitor_info(const_cast<MonitorInfo&>(Monitor::info()));
                engine_instance->rhi()->init_window(_M_interface, global_window_config);
                info_log("TrinexEngine", "Selected GPI: %s", engine_instance->rhi()->renderer().c_str());

                engine_instance->_M_window =
                        Object::new_instance_named<Window>("MainWindow", Package::find_package("Engine"), _M_interface);

                AfterRHIInitializeController().execute();

                info_log("Window", "End creating window");
                return sizeof(CreateWindowTask);
            }
        };


        WindowInterface* (*loader)() = library.get<WindowInterface*>(Constants::library_load_function_name);
        if (!loader)
        {
            throw EngineException("Cannot load window system loader");
        }

        global_window_config.update();
        global_window_config.api_name = engine_config.api;

        thread(ThreadType::RenderThread)->insert_new_task<CreateWindowTask>(loader);
        thread(ThreadType::RenderThread)->wait_all();

        if (engine_config.use_deffered_rendering)
        {
            GBuffer::create_instance();
        }

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

    int_t EngineInstance::launch_systems()
    {

        EngineSystem* engine_system = System::new_system<EngineSystem>();

        if (engine_system->subsystems().empty())
        {
            error_log("Engine", "No systems found! Please, add systems before call '%s' method!", __FUNCTION__);
            return -1;
        }

        try
        {
            static constexpr float smoothing_factor = 0.05;

            float prev_time    = 0.0167;
            float current_time = 0.0f;
            float dt           = 0.0f;

            while (!is_requesting_exit())
            {
                current_time = time_seconds();
                dt           = smoothing_factor * (current_time - prev_time) + (1 - smoothing_factor) * dt;
                prev_time    = current_time;

                engine_system->update(dt);
                engine_system->wait();
                ++_M_frame_index;
            }

            engine_system->shutdown();

            return 0;
        }
        catch (const std::exception& e)
        {
            error_log("Engine", "%s", e.what());
            return -1;
        }
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
