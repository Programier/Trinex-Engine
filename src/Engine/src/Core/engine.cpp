#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/file_manager.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/shader_compiler.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <Graphics/renderer.hpp>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <cstring>
#include <glm/gtc/quaternion.hpp>
#include <no_api.hpp>

namespace Engine
{
    extern void trinex_init_sdl();
    extern void trinex_terminate_sdl();

    Vector<void (*)()>& terminate_list()
    {
        static Vector<void (*)()> _M_terminate_list;
        return _M_terminate_list;
    }

    Vector<void (*)()>& initialize_list()
    {
        static Vector<void (*)()> _M_init_list;
        return _M_init_list;
    }

    static Vector<void (*)()>& preinitialize_list()
    {
        static Vector<void (*)()> _M_init_list;
        return _M_init_list;
    }

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

    const Window* EngineInstance::window() const
    {
        static Window window;
        return &window;
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
        else if (name == "OpenGL")
            return EngineAPI::OpenGL;

        return EngineAPI::NoAPI;
    }

    EngineInstance& EngineInstance::init_api()
    {
        if (_M_api != EngineAPI::NoAPI)
        {
            trinex_init_sdl();

            Library api_library(engine_config.api.c_str());
            info_log("Engine", "Using API: %s", engine_config.api.c_str());

            if (!api_library.has_lib())
            {
                throw EngineException("Failed to load API library!");
            }

            // Try to load api loader
            GraphicApiInterface::ApiInterface* (*loader)() =
                    api_library.get<GraphicApiInterface::ApiInterface*>("load_api");

            if (!loader)
            {
                throw EngineException("Failed to get API loader!");
            }

            // Initialize API
            _M_api_interface = loader();

            if (!_M_api_interface)
            {
                throw EngineException("Failed to init API");
            }
        }
        else
        {
            _M_api_interface = new NoApi();
        }
        return *this;
    }

    EngineInstance* EngineInstance::_M_instance   = nullptr;
    ENGINE_EXPORT EngineInstance* engine_instance = nullptr;


    static CommandLet* try_load_commandlet(const String& name, Class* base_class)
    {
        Class* class_instance = Class::find_class(name);
        if (!class_instance)
        {
            logger->error("Engine", "Failed to load commandlet '%s'", name.c_str());
            return nullptr;
        }

        if (!class_instance->contains_class(base_class))
        {
            error_log("Engine", "Class '%s' does not inherit from class Engine::CommandLet!",
                      class_instance->name().c_str());
            return nullptr;
        }

        Object* object         = class_instance->create();
        CommandLet* commandlet = object->instance_cast<CommandLet>();

        if (!commandlet)
        {
            logger->error("Engine", "Class '%s' is not commandlet!", class_instance->name().c_str());
        }

        return commandlet;
    }

    static CommandLet* find_commandlet(int argc, char** argv)
    {
        Class* commandlet_base_class = Class::find_class("Engine::CommandLet");
        CommandLet* commandlet       = nullptr;
        // Load commandlet
        if (argc > 1)
        {
            commandlet = try_load_commandlet(argv[1], commandlet_base_class);
        }

        if (!commandlet)
        {
            commandlet = try_load_commandlet(engine_config.base_commandlet, commandlet_base_class);
        }

        return commandlet;
    }

    int EngineInstance::start(int argc, char** argv)
    {
        logger->log("TrinexEngine", "Start engine!");
        if (is_inited())
        {
            return -1;
        }

        for (auto preinit_callback : preinitialize_list())
        {
            preinit_callback();
        }

        preinitialize_list().clear();


        FileManager* root_manager = const_cast<FileManager*>(FileManager::root_file_manager());

#if PLATFORM_ANDROID
        root_manager->work_dir(Strings::format("/sdcard/TrinexGames/{}/", EngineInstance::project_name()));
#else
        if (argc > 0)
        {
            root_manager->work_dir(FileManager::dirname_of(argv[0]));
        }
#endif

        Lua::Interpretter::init();

        for (auto func : initialize_list())
        {
            func();
        }

        initialize_list().clear();

        logger->log("EngineInstance", "Work dir is '%s'", root_manager->work_dir().c_str());
        engine_config.load_config((root_manager->work_dir() / Path("TrinexEngine/configs/init_config.cfg")).string());

        Lua::Interpretter::init_lua_dir();

        CommandLet* commandlet = find_commandlet(argc, argv);
        if (!commandlet)
        {
            return -1;
        }

        commandlet->on_config_load();

        _M_api = get_api_by_name(engine_config.api);

        init_api();

        _M_renderer = new Renderer(_M_api_interface);
        _M_api_interface->logger(logger);
        Monitor::update();
        Sensor::update_sensors_info();
        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsInited)] = true;

        if (engine_config.max_g_buffer_height < 200)
        {
            engine_config.max_g_buffer_height = static_cast<uint_t>(Monitor::height());
        }

        if (engine_config.max_g_buffer_width < 200)
        {
            engine_config.max_g_buffer_width = static_cast<uint_t>(Monitor::width());
        }

        auto status = commandlet->execute(argc - 1, argv + 1);
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

    GraphicApiInterface::ApiInterface* EngineInstance::api_interface() const
    {
        return _M_api_interface;
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
    ja stack_address
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
        return is_on_stack_asm(ptr);
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
        for (auto& func : terminate_list())
        {
            func();
        }

        return *this;
    }

    EngineInstance::~EngineInstance()
    {
        request_exit();
        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsShutingDown)] = true;
        info_log("EngineInstance", "Terminate Engine");

        for (Thread*& thread : _M_threads)
        {
            delete thread;
            thread = nullptr;
        }

        engine_instance->trigger_terminate_functions();
        Object::force_garbage_collection();

        Lua::Interpretter::terminate();

        delete _M_renderer;
        delete _M_api_interface;
        trinex_terminate_sdl();

        _M_api_interface                                                    = nullptr;
        _M_flags[static_cast<EnumerateType>(EngineInstanceFlags::IsInited)] = false;
        Library::close_all();
    }

    bool EngineInstance::check_format_support(PixelType type, PixelComponentType component)
    {
        return _M_api_interface->check_format_support(type, component);
    }


    static const char* thread_name(ThreadType type)
    {
        switch (type)
        {
            case ThreadType::RenderThread:
                return "Render Thread";
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


    /////////////////// DESTROY CONTROLLER ///////////////////

    DestroyController::DestroyController(void (*callback)())
    {
        terminate_list().push_back(callback);
    }

    InitializeController::InitializeController(void (*callback)())
    {
        initialize_list().push_back(callback);
    }

    PreInitializeController::PreInitializeController(void (*callback)())
    {
        preinitialize_list().push_back(callback);
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
                logger->error("TrinexEngine", "%s", e.what());
                result = -1;
            }

            engine_instance->destroy();
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
