#include <Core/engine.hpp>
#include <Core/init.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <LibLoader/lib_loader.hpp>
#include <SDL.h>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <Window/window.hpp>
#include <api_funcs.hpp>
#include <opengl.hpp>
#include <thread>


namespace Engine
{
    ENGINE_EXPORT OpenGL_Version_S OpenGL_Version{3, 0};
    static bool engine_init_status = false;
    static EngineAPI Engine_API_value;
    ENGINE_EXPORT Init init;
    extern std::vector<std::pair<void**, const char*>> _M_extern_funcs;
    extern bool (*api_init)(std::vector<int> params);
    extern void (*api_terminate)();

#if defined(WIN32)
    const SystemName system_name = SystemName::WINDOWS_OS;
#elif defined(__ANDROID__)
    const SystemName system_name = SystemName::ANDROID_OS;
#else
    const SystemName system_name = SystemName::LINUX_OS;
#endif

    static void Init_SDL_Lib()
    {
        if (SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO))
            throw std::runtime_error(SDL_GetError());

        if (Engine_API_value == EngineAPI::OpenGL)
        {}
    }

    ENGINE_EXPORT bool is_inited()
    {
        return engine_init_status;
    }


    void Init::operator()(const EngineAPI& API)
    {
        if (engine_init_status)
        {
            logger->log("Engine: Failed to init engine. Engine already inited\n");
            return;
        }

        Engine_API_value = API;

        logger->log("Engine: Start init Engine\n");


        const char* libname = (API == EngineAPI::OpenGL ? "OpenGL" : "Vulkan");
        Library api_lib = load_library(libname);

        if (!api_lib.has_lib())
            throw std::runtime_error("Failed to load api " + std::string(libname));

        for (auto& pair : _M_extern_funcs) (*pair.first) = (void*) api_lib.get<void>(pair.second);

        Init_SDL_Lib();

        if (!api_init || !api_init({}))
            throw std::runtime_error("Failed to init " + std::string(libname));

        Monitor::update();
        Sensor::update_sensors_info();
        set_logger(Engine::logger);
        logger->log("Engine: Init done\n");
        engine_init_status = true;
    }

    ENGINE_EXPORT void except_init_check()
    {
        if (!engine_init_status)
            throw std::runtime_error("Engine: Engine is not inited. Please, init Engine first");
    }

    ENGINE_EXPORT const EngineAPI& Engine_API()
    {
        return Engine_API_value;
    }


    static class EngineTerminateController
    {
    public:
        ~EngineTerminateController()
        {
            if (!engine_init_status)
                return;

            logger->log("Engine: Terminate\n");
            Window::close();
            api_terminate();
            SDL_Quit();
            engine_init_status = false;
        }
    } Controller;

}// namespace Engine