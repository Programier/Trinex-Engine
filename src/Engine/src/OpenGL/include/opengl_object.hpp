#pragma once
#ifdef _WIN32
#include <GL/glew.h>
////////////////////////
#include <GL/gl.h>

#if !defined(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) && defined(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#endif

#if !defined(GL_DEPTH_CLAMP_EXT) && defined(GL_DEPTH_CLAMP)
#define GL_DEPTH_CLAMP_EXT GL_DEPTH_CLAMP
#endif

#else
#include <GLES3/gl32.h>
////////////////////////
#include <GLES2/gl2ext.h>

#if !defined(GL_DEPTH_CLAMP_EXT) && defined (GL_DEPTH_CLAMP)
    #define GL_DEPTH_CLAMP_EXT GL_DEPTH_CLAMP
#endif

#endif

#include <Core/engine_types.hpp>

#define object_of(id) reinterpret_cast<OpenGL_Object*>(id)
#define GET_TYPE(type, id) reinterpret_cast<type*>(object_of(id)->get_instance_data())
#define implement_opengl_instance_hpp() void* get_instance_data() override
#define implement_opengl_instance_cpp(name)                                                                            \
    void* name::get_instance_data()                                                                                    \
    {                                                                                                                  \
        return this;                                                                                                   \
    }

namespace Engine
{
    struct OpenGL_Object {
        virtual void* get_instance_data() = 0;
        GLuint _M_instance_id             = 0;

        inline Identifier ID()
        {
            return reinterpret_cast<Identifier>(this);
        }

        virtual ~OpenGL_Object()
        {}
    };
}// namespace Engine
