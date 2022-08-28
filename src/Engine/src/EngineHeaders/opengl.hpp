#pragma once
#include <BasicFunctional/engine_types.hpp>

#define __ANDROID__

#ifdef __ANDROID__
#include <GLES3/gl32.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

namespace Engine
{
    namespace OpenGL
    {
        int get_buffer(const BufferType& buffer);
    }
}
