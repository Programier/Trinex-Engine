#include <Core/destroy_controller.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <LibLoader/lib_loader.hpp>
#include <SDL.h>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <api.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

namespace Engine
{
    static std::vector<void (*)()>& terminate_list()
    {
        static std::vector<void (*)()> _M_terminate_list;
        return _M_terminate_list;
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
        return SystemName::WindowsOS;
#elif defined(__ANDROID__)
        return SystemName::AndroidOS;
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


    static std::string get_api_name(EngineAPI api)
    {
        switch (api)
        {
            case EngineAPI::OpenGL:
                return "OpenGL";

            case EngineAPI::Vulkan:
                return "Vulkan";
            default:
                return "API_NOT_FOUND!";
        }
    }

    EngineInstance& EngineInstance::init()
    {
        if (_M_is_inited)
        {
            return *this;
        }

        if (SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO))
            throw std::runtime_error(SDL_GetError());


        String api_name = get_api_name(_M_api);
        Library api_library = load_library(api_name);
        logger->log("Engine: Using API: %s", api_name.c_str());

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

        _M_api_interface->logger(logger);
        Monitor::update();
        Sensor::update_sensors_info();

        _M_is_inited = true;
        return *this;
    }

    GraphicApiInterface::ApiInterface* EngineInstance::api_interface() const
    {
        return _M_api_interface;
    }

    EngineInstance& EngineInstance::enable(EnableCap cap)
    {
        _M_api_interface->enable(cap);
        return *this;
    }

    EngineInstance& EngineInstance::disable(EnableCap cap)
    {
        _M_api_interface->disable(cap);
        return *this;
    }

    EngineInstance& EngineInstance::blend_func(BlendFunc func, BlendFunc func2)
    {
        _M_api_interface->blend_func(func, func2);
        return *this;
    }

    EngineInstance& EngineInstance::depth_func(DepthFunc func)
    {
        _M_api_interface->depth_func(func);
        return *this;
    }

    EngineInstance& EngineInstance::depth_mask(bool mask)
    {
        _M_api_interface->depth_mask(mask);
        return *this;
    }

    EngineInstance& EngineInstance::stencil_mask(byte mask)
    {
        _M_api_interface->stencil_mask(mask);
        return *this;
    }

    EngineInstance& EngineInstance::stencil_option(StencilOption stencil_fail, StencilOption depth_fail,
                                                   StencilOption pass)
    {
        _M_api_interface->stencil_option(stencil_fail, depth_fail, pass);
        return *this;
    }

    EngineInstance& EngineInstance::stencil_func(Engine::CompareFunc func, int_t ref, byte mask)
    {
        _M_api_interface->stencil_func(func, ref, mask);
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
        logger->log("Engine: Terminate Engine");

        delete _M_api_interface;
        SDL_Quit();

        _M_api_interface = nullptr;
        _M_is_inited = false;
    }


    /////////////////// DESTROY CONTROLLER ///////////////////

    DestroyController::DestroyController(void (*callback)())
    {
        terminate_list().push_back(callback);
    }
}// namespace Engine
