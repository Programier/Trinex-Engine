#ifdef _WIN32
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <iostream>
#include <opengl_object.hpp>


static void* _M_context = nullptr;

API void api_destroy_window()
{
    if (_M_context)
        SDL_GL_DeleteContext(_M_context);
    _M_context = nullptr;
}

API void* api_init_window(SDL_Window* window)
{
    if (_M_context)
        return _M_context;
    _M_context = SDL_GL_CreateContext(window);

    if (!_M_context)
    {
        external_logger->log("Failed to create OpenGL context: %s\n", SDL_GetError());
        return nullptr;
    }

    SDL_GL_MakeCurrent(window, _M_context);
    external_logger->log("Context address: %p\n", _M_context);

#ifdef _WIN32
    external_logger->log("Start init glew\n");
    auto status = glewInit();
    if (status != GLEW_OK)
    {
        api_destroy_window();
        external_logger->log("Failed to init glew: %s\n", glewGetErrorString(status));
    }
#endif

    return _M_context;
}
