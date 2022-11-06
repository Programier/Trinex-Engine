#pragma once
#include <Core/logger.hpp>
#include <cstddef>
#include <opengl_export.hpp>
#include <opengl_types.hpp>


class OpenGL_Object
{
public:
    std::size_t _M_references = 1;
    OpenGL_Object();
    virtual ~OpenGL_Object();
    virtual void destroy();
};


template<typename ObjectType = OpenGL_Object>
ObjectType* object_of(ObjID ID)
{
    OpenGL_Object* object = reinterpret_cast<OpenGL_Object*>(ID);
    return dynamic_cast<ObjectType*>(object);
}

ObjID object_id_of(const OpenGL_Object* object);


#define API extern "C" OPENGL_EXPORT

#define check_id(id, out)                                                                                                   \
    if (!id)                                                                                                                \
    return out

#define check check_id
API void api_destroy_object_instance(ObjID& ID);

extern Engine::Logger* external_logger;
int get_current_binding(GLint obj);
