#include <Init/init.hpp>
#include <SDL.h>
#include <Window/window.hpp>
#include <engine.hpp>
#include <opengl.hpp>
#include <thread>

#include <iostream>

namespace Engine
{
    OpenGL_Version_S OpenGL_Version{3, 0};
    static bool engine_init_status = false;
    static EngineAPI Engine_API;


    // Monitor init
    namespace Monitor
    {
        SDL_DisplayMode mode;

        void update()
        {
            SDL_GetCurrentDisplayMode(0, &mode);
        }


        int red_bits()
        {
            throw not_implemented;
        }

        int green_bits()
        {
            throw not_implemented;
        }

        int blue_bits()
        {
            throw not_implemented;
        }

        Size1D height()
        {
            return static_cast<Size1D>(mode.h);
        }

        Size1D width()
        {
            return static_cast<Size1D>(mode.w);
        }

        int refresh_rate()
        {
            return mode.refresh_rate;
        }

        Size2D size()
        {
            return {width(), height()};
        }
    }// namespace Monitor

    // API initialization
    static void open_gl_init()
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OpenGL_Version.major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OpenGL_Version.minor);
#ifdef __ANDROID__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifndef __ANDROID__
        glewExperimental = true;
#endif
    }

    static void vulkan_init()
    {
        throw not_implemented;
    }

    static void (*init_api[2])() = {open_gl_init, vulkan_init};

    // API terminate

    static void open_gl_terminate()
    {
        SDL_Quit();
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

        if (SDL_Init(SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO))
            throw std::runtime_error(SDL_GetError());

        init_api[static_cast<unsigned int>(API)]();
        Monitor::update();
        std::clog << "Engine: Init done" << std::endl;
        engine_init_status = true;
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
            Window::close();
            termiante_api[static_cast<unsigned int>(Engine_API)]();
            engine_init_status = false;
        }
    } Controller;

}// namespace Engine
