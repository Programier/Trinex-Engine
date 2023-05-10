#include <Core/commandlet.hpp>
#include <Core/config.hpp>
#include <Core/destroy_controller.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Graphics/renderer.hpp>
#include <LibLoader/lib_loader.hpp>
#include <SDL.h>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <api.hpp>
#include <cstring>
#include <glm/gtc/quaternion.hpp>


namespace Engine
{
    Vector<void (*)()>& terminate_list()
    {
        static Vector<void (*)()> _M_terminate_list;
        return _M_terminate_list;
    }

    Vector<void (*)()>& initialize_list()
    {
        static Vector<void (*)()> init_list;
        return init_list;
    }

    EngineInstance::EngineInstance()
    {}


    EngineInstance* EngineInstance::_M_instance = nullptr;

    ENGINE_EXPORT EngineInstance* EngineInstance::create_instance()
    {
        if (_M_instance == nullptr)
            _M_instance = new EngineInstance();
        return _M_instance;
    }

    ENGINE_EXPORT EngineInstance* EngineInstance::instance()
    {
        if (_M_instance == nullptr)
            _M_instance = new EngineInstance();
        return _M_instance;
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
            if (SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO))
                throw std::runtime_error(SDL_GetError());

            Library api_library = load_library(engine_config.api.c_str());
            logger->log("Engine: Using API: %s", engine_config.api.c_str());

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

    int EngineInstance::start(int argc, char** argv)
    {
        if (_M_is_inited)
        {
            return -1;
        }
        FileManager* root_manager = const_cast<FileManager*>(FileManager::root_file_manager());

        if (argc > 0)
        {
            root_manager->work_dir(FileManager::dirname_of(argv[0]));
        }

        for (auto func : initialize_list())
        {
            func();
        }

        initialize_list().clear();

        engine_config.init("config.cfg");

        _M_api = get_api_by_name(engine_config.api);

        init_api();

        _M_renderer = new Renderer(_M_api_interface);
        _M_api_interface->logger(logger);
        Monitor::update();
        Sensor::update_sensors_info();
        _M_is_inited = true;

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
                    return -1;
                }
            }
            else
            {
                logger->error("Engine: Failed to load commandlet '%s'", argv[1]);
                return -1;
            }
        }

        if (!command_let)
        {
            Class* class_instance = Class::find_class(engine_config.base_commandlet);

            if (!class_instance)
            {
                logger->error("Engine: Failed to load commandlet '%s'", engine_config.base_commandlet.c_str());
                return -1;
            }

            Object* object = class_instance->create();
            command_let    = object->instance_cast<CommandLet>();

            if (!command_let)
            {
                logger->error("Engine: Class '%s' is not commandlet!", engine_config.base_commandlet.c_str());
                return -1;
            }
        }

        auto status = command_let->execute(argc - 1, argv + 1);
        if (status == 0)
        {
            logger->log("Engine: Commandlet execution success!");
        }
        else
        {
            logger->log("Engine: Failed to execute commandlet. Error code: %d", status);
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
        logger->log("Engine: Terminate Engine");
        EngineInstance::_M_instance->trigger_terminate_functions();

        Window::destroy_window();

        delete _M_renderer;
        delete _M_api_interface;
        SDL_Quit();

        _M_api_interface = nullptr;
        _M_is_inited     = false;
    }

    bool EngineInstance::check_format_support(PixelType type, PixelComponentType component)
    {
        return _M_api_interface->check_format_support(type, component);
    }

    void EngineInstance::destroy()
    {
        delete _M_instance;
        _M_instance = nullptr;
    }


    /////////////////// DESTROY CONTROLLER ///////////////////

    DestroyController::DestroyController(void (*callback)())
    {
        terminate_list().push_back(callback);
    }
}// namespace Engine
