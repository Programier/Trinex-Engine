#pragma once
#include <opengl_headers.hpp>
#include <Core/enums.hpp>

namespace Engine
{
    struct OpenGL_ColorInfo {
        GLenum m_internal_format;
        GLenum m_format;
        GLenum m_type;

        OpenGL_ColorInfo(GLenum internal_format = 0, GLenum format = 0, GLenum type = 0)
            : m_internal_format(internal_format), m_format(format), m_type(type)
        {}

        FORCE_INLINE bool is_valid() const
        {
            return m_internal_format != 0 && m_format != 0 && m_type != 0;
        }
    };

    OpenGL_ColorInfo color_format_from_engine_format(ColorFormat format);
}// namespace Engine
