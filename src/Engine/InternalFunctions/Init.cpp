#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Init/init.hpp>
#include <engine.hpp>
#include <thread>

#include <iostream>

namespace Engine
{

    static bool engine_init_status = false;
    static EngineAPI Engine_API;


    // Monitor init
    namespace Monitor
    {
        GLFWmonitor* _M_monitor;
        GLFWvidmode videomode;
        bool inited;

        static void update();

        static void monitor_callback(GLFWmonitor* _monitor, int value)
        {

            if (value)
            {
                if (_monitor != _M_monitor)
                    init();
                else
                    update();
            }
            else
            {
                std::cerr << "MONITOR: Monitor dissconected" << std::endl;
                inited = false;
            }
        }

        static void init()
        {
            std::clog << "MONITOR: Reading Monitor data" << std::endl;
            _M_monitor = glfwGetPrimaryMonitor();
            if (_M_monitor == nullptr)
            {
                std::cerr << "MONITOR: Failed to read Monitor data" << std::endl;
                return;
            }

            update();
            glfwSetMonitorCallback(monitor_callback);
            inited = true;
        }

        static void update()
        {
            auto _vidmode = glfwGetVideoMode(_M_monitor);
            if (_vidmode == nullptr)
            {
                _M_monitor = nullptr;
                std::clog << "MONITOR: Failed to read Monitor data" << std::endl;
                return;
            }

            videomode = *_vidmode;
        }

        int red_bits()
        {
            return inited ? videomode.redBits : -1;
        }

        int green_bits()
        {
            return inited ? videomode.greenBits : -1;
        }

        int blue_bits()
        {
            return inited ? videomode.blueBits : -1;
        }

        Size1D height()
        {
            return inited ? static_cast<Size1D>(videomode.height) : -1.f;
        }

        Size1D width()
        {
            return inited ? static_cast<Size1D>(videomode.width) : -1.f;
        }

        int refresh_rate()
        {
            return inited ? videomode.refreshRate : -1;
        }

        void* monitor()
        {
            return reinterpret_cast<void*>(_M_monitor);
        }

        Size2D size()
        {
            return {width(), height()};
        }
    }// namespace Monitor

    // API initialization
    static void open_gl_init()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glewExperimental = GL_TRUE;

        // Init glfw
        engine_init_status = (glfwInit() == 1);
        if (engine_init_status)
            std::clog << "Engine: GLFW init complete" << std::endl;
        else
            throw std::runtime_error("Engine: Failed to init Engine");
    }

    static void vulkan_init()
    {
        throw not_implemented;
    }

    static void (*init_api[2])() = {open_gl_init, vulkan_init};

    // API terminate

    static void open_gl_terminate()
    {
        glfwTerminate();
    }

    static void vulkan_terminate()
    {
        throw not_implemented;
    }

    static void (*termiante_api[2])() = {open_gl_terminate, vulkan_terminate};


    bool is_inited()
    {
        return engine_init_status;
    }


    void init(const EngineAPI& API)
    {
        if (engine_init_status)
        {
            std::clog << "Engine: Failed to init engine. Engine already inited" << std::endl;
            return;
        }

        Engine_API = API;
        std::clog << "Engine: Start init Engine" << std::endl;
        init_api[static_cast<unsigned int>(API)]();
        std::clog << "Engine: Init done" << std::endl;
        Monitor::init();
    }

    void except_init_check()
    {
        if (!engine_init_status)
            throw std::runtime_error("Engine: Engine is not inited. Please, init Engine first");
    }

    const EngineAPI& API()
    {
        return Engine_API;
    }


    static class EngineTerminateController
    {
    public:
        ~EngineTerminateController()
        {
            if (!engine_init_status)
                return;
            std::clog << "Engine: Terminate" << std::endl;
            termiante_api[static_cast<unsigned int>(Engine_API)]();
            engine_init_status = false;
        }
    } Controller;

}// namespace Engine
