#include <GL/glew.h>
#include <engine.hpp>
#include <opengl.hpp>


using namespace Engine;


int Engine::OpenGL::get_buffer(const BufferType& buffer)
{
    int opengl_buffer = 0;
    if ((buffer & COLOR_BUFFER_BIT) == COLOR_BUFFER_BIT)
        opengl_buffer |= GL_COLOR_BUFFER_BIT;

    if ((buffer & DEPTH_BUFFER_BIT) == DEPTH_BUFFER_BIT)
        opengl_buffer |= GL_DEPTH_BUFFER_BIT;
    return opengl_buffer;
}
