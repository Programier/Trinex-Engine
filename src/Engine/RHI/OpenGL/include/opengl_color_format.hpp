#pragma once
#include <Core/color_format.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_ColorInfo {
        GLenum _M_internal_format;
        GLenum _M_format;
        GLenum _M_type;

        OpenGL_ColorInfo(GLenum internal_format = 0, GLenum format = 0, GLenum type = 0)
            : _M_internal_format(internal_format), _M_format(format), _M_type(type)
        {}

        FORCE_INLINE bool is_valid() const
        {
            return _M_internal_format != 0 && _M_format != 0 && _M_type != 0;
        }
    };

    OpenGL_ColorInfo color_format_from_engine_format(ColorFormat format);
}// namespace Engine
