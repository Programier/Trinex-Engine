#pragma once
#include <cstddef>
#include <opengl_types.hpp>
#include <Core/logger.hpp>
#include <opengl_export.hpp>



enum class Type : std::size_t
{
    UNDEFINED = 0,
    TEXTURE = 1,
    MESH = 2,
    FRAMEBUFFER = 3,
};


struct OpenGL_Object {
    Type _M_type = Type::UNDEFINED;
    std::size_t _M_references = 0;
    void* _M_data = nullptr;
};

#define object_of(id) reinterpret_cast<OpenGL_Object*>(id)

#define API extern "C" OPENGL_EXPORT

#define check_id(id, out)                                                                                                   \
    if (!id)                                                                                                                \
    return out

API void api_destroy_object_instance(ObjID& ID);

Engine::Logger* get_external_logger();
int get_current_binding(GLint obj);
