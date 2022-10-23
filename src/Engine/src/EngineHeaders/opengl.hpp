#pragma once
#include <Core/engine_types.hpp>
#ifdef _WIN32
#include <GL/glew.h>

#include <GL/gl.h>
#else
#include <GLES3/gl32.h>
#endif

namespace Engine
{
    namespace OpenGL
    {
        int get_buffer(const BufferType& buffer);
    }
}
