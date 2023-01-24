#include <Core/logger.hpp>

#ifdef _WIN32
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <iostream>
#include <opengl.hpp>
#include <opengl_object.hpp>
#include <vector>


#ifdef ENGINE_DEBUG
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                    const void* userParam)
{
    external_logger->log("%s\n", message);
}
#endif

API bool api_init(std::vector<int> params)
{
    external_logger->log("OpenGL API: Start init API!\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#ifdef _WIN32
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    return true;
}

API void api_terminate()
{}
