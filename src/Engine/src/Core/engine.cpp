#include <Core/commandlet.hpp>
#include <Core/config.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/predef.hpp>
#include <Graphics/renderer.hpp>
#include <LibLoader/lib_loader.hpp>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <api.hpp>
#include <cstring>
#include <glm/gtc/quaternion.hpp>


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
    {}

    ENGINE_EXPORT EngineInstance* EngineInstance::instance()
    {
        if (engine_instance == nullptr)
            engine_instance = new EngineInstance();
        return engine_instance;
    }

    ENGINE_EXPORT const String& EngineInstance::project_name()
    {
        static String name;
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

    SystemType EngineInstance::system_type() const
    {
#if defined(WIN32)
        return SystemType::WindowsOS;
#elif defined(__ANDROID__)
        return SystemType::AndroidOS;
#else
        return SystemType::LinuxOS;
#endif
    }

    EngineAPI EngineInstance::api() const
    {
        return _M_api;
    }


    /////////////////// INITIALIZE ENGINE ///////////////////

    bool EngineInstance::is_inited() const
    {
        return _M_is_inited;
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

            LibraryLoader::Library api_library = LibraryLoader::load(engine_config.api.c_str());
            info_log("Engine: Using API: %s", engine_config.api.c_str());

            if (!api_library.has_lib())
            {
                throw std::runtime_error("Engine: Failed to load API library!");
            }

            // Try to load api loader
            GraphicApiInterface::ApiInterface* (*loader)() =
                    api_library.get<GraphicApiInterface::ApiInterface*>("load_api");

            if (!loader)
            {
                throw std::runtime_error("Engine: Failed to get API loader!");
            }

            // Initialize API
            _M_api_interface = loader();

            if (!_M_api_interface)
            {
                throw std::runtime_error("Engine: Failed to init API");
            }
        }
        else
        {
            _M_api_interface = new GraphicApiInterface::ApiInterface();
        }
        return *this;
    }

    ENGINE_EXPORT EngineInstance* engine_instance;


    static CommandLet* find_command_let(int argc, char** argv)
    {
        // Load commandlet
        CommandLet* command_let = nullptr;
        if (argc > 1)
        {
            Class* class_instance = Class::find_class(argv[1]);
            if (class_instance)
            {
                Object* object = class_instance->create();
                command_let    = object->instance_cast<CommandLet>();

                if (!command_let)
                {
                    logger->error("Engine: Class '%s' is not commandlet!", class_instance->name().c_str());
                    return nullptr;
                }
            }
            else
            {
                logger->error("Engine: Failed to load commandlet '%s'", argv[1]);
                return nullptr;
            }
        }

        if (!command_let)
        {
            Class* class_instance = Class::find_class(engine_config.base_commandlet);

            if (!class_instance)
            {
                logger->error("Engine: Failed to load commandlet '%s'", engine_config.base_commandlet.c_str());
                return nullptr;
            }

            Object* object = class_instance->create();
            command_let    = object->instance_cast<CommandLet>();

            if (!command_let)
            {
                logger->error("Engine: Class '%s' is not commandlet!", engine_config.base_commandlet.c_str());
                return nullptr;
            }
        }

        return command_let;
    }

    int EngineInstance::start(int argc, char** argv)
    {
        if (_M_is_inited)
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

        logger->log("Engine: Work dir is '%s'", root_manager->work_dir().c_str());
        engine_config.init((root_manager->work_dir() / Path("TrinexEngine/configs/init_config.cfg")).string());

        logger->log("Engine: Work dir is '%s'", root_manager->work_dir().string().c_str());

        Lua::Interpretter::init_lua_dir();

        CommandLet* command_let = find_command_let(argc, argv);
        if (!command_let)
        {
            return -1;
        }

        command_let->on_config_load();

        _M_api = get_api_by_name(engine_config.api);

        init_api();

        _M_renderer = new Renderer(_M_api_interface);
        _M_api_interface->logger(logger);
        Monitor::update();
        Sensor::update_sensors_info();
        _M_is_inited = true;

        if (engine_config.max_g_buffer_height < 200)
        {
            engine_config.max_g_buffer_height = static_cast<uint_t>(Monitor::height());
        }

        if (engine_config.max_g_buffer_width < 200)
        {
            engine_config.max_g_buffer_width = static_cast<uint_t>(Monitor::width());
        }

        auto status = command_let->execute(argc - 1, argv + 1);
        if (status == 0)
        {
            info_log("Engine: Commandlet execution success!");
        }
        else
        {
            info_log("Engine: Failed to execute commandlet. Error code: %d", status);
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

    bool EngineInstance::is_on_stack(void* ptr)
    {
        return is_on_stack_asm(ptr);
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
        Lua::Interpretter::terminate();

        info_log("Engine: Terminate Engine");
        engine_instance->trigger_terminate_functions();

        Window::destroy_window();

        delete _M_renderer;
        delete _M_api_interface;
        trinex_terminate_sdl();

        _M_api_interface = nullptr;
        _M_is_inited     = false;
    }

    bool EngineInstance::check_format_support(PixelType type, PixelComponentType component)
    {
        return _M_api_interface->check_format_support(type, component);
    }

    void EngineInstance::destroy()
    {
        delete engine_instance;
        engine_instance = nullptr;
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
        EngineInstance* instance = EngineInstance::instance();
        if (!instance->_M_is_inited)
        {
            auto result = instance->start(argc, argv);
            instance->destroy();
            return result;
        }

        return -1;
    }


}// namespace Engine

ENGINE_EXPORT int main(int argc, char** argv)
{
    return Engine::EngineInstance::initialize(argc, argv);
}
