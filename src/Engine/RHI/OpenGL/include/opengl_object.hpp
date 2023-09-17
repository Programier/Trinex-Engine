#pragma once
#include <opengl_headers.hpp>

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
