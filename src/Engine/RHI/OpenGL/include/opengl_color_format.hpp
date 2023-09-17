#pragma once
#include <Core/texture_types.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_ColorFormat {
        GLuint internal_format;
        GLuint format;
        GLuint type;


        static OpenGL_ColorFormat from(ColorFormat format);
    };

}// namespace Engine
